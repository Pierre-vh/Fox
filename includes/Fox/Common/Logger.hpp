//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See the LICENSE.txt file at the root of the project for license information.            
// File : Logger.hpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//
// This file contains a Logger class which is a class that can be used to 
// easily add customisable logging utilities to a class.
//----------------------------------------------------------------------------//

#pragma once

#include <functional> // reference_wrapper
#include <iostream> // std::ostream, cout
#include "Errors.hpp"

namespace fox {
  class Logger {
    public:
      class LogPrinter;
      class IndentGuard;

      Logger(std::ostream& os = std::clog);

      // Logging Functions //

      // Traditional logging, works just like std::cout
      LogPrinter& operator()(std::int8_t indent = 0);

      std::ostream& getOS();
      void setOS(std::ostream& os);

      void setIndentStr(const std::string& str);
      std::string getIndentStr();

      void enable();
      void disable();
      bool isEnabled();

      void indent(std::int8_t val = 1);
      // Indents by creating a RAII IndentGuard object, which will automatically
      // remove the indent upon destruction.
      IndentGuard indentGuard(std::int8_t val = 1);
      void dedent(std::int8_t val = 1);

      // Signals the logger that we're entering a function. This will also create
      // an IndentGuard with a indentation of 1 to indent every logs emitted during
      // the function's execution
      template<typename ... Args>
      IndentGuard enterFunc(const std::string& name, Args&& ... args) {
        getOS() << getIndent() << name << '(';
        printArg(std::forward<Args>(args)...);
        getOS() << ")\n";
        return IndentGuard(this, 1);
      }

      // The class which implements the << operator overloads and
      // handle the printing.
      class LogPrinter {
        Logger* logger_ = nullptr;
        public:
          template<typename Ty>
          LogPrinter& operator<<(Ty&& val) {
            // Only do it if the logger is enabled.
            if (logger_->isEnabled())
              logger_->getOS() << val;
            return (*this);
          }

        protected:
          friend class Logger;
          LogPrinter(Logger* logger) : logger_(logger) {
            assert(logger && "Logger instance cannot be null!");
          }
      };

      // A RAII indentation guardian, which is going to indent in
      // the constructor and dedent in the destructor.
      class IndentGuard {
        Logger* logger_ = nullptr;
        std::int8_t val_ = 0;
        protected:
          friend class Logger;
          IndentGuard(Logger* logger, std::int8_t indent):
            logger_(logger), val_(indent) {
            assert(logger && "Logger instance cannot be null!");
            logger_->indent(val_);
          }
        public:
          ~IndentGuard() {
            logger_->dedent(val_);
          }
      };

    private:
      template<typename Ty, typename ... Args>
      void printArg(Ty&& arg, Args&& ... args) {
        getOS() << arg << ", ";
        printArg(std::forward<Args>(args)...);
      }

      template<typename Ty>
      void printArg(Ty&& arg) {
        getOS() << arg;
      }

      void printArg() {}

      std::string getIndent(std::int8_t additionalIndent = 0);

      bool enabled_ = false;
      std::reference_wrapper<std::ostream> outRW_;
      LogPrinter printer_;
      std::int16_t indentDepth_ = 0;
      std::string indentStr_ = " |";
  };
}
