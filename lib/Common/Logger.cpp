//----------------------------------------------------------------------------//
// This file is a part of The Moonshot Project.        
// See LICENSE.txt for license info.            
// File : Logger.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/Common/Logger.hpp"
#include <string>

using namespace fox;

Logger::Logger(std::ostream& os) : outRW_(os), printer_(this) {
  // Enable the logging by default in debug mode
  #ifndef NDEBUG
    enabled_ = true;
  #else
    enabled_ = false;
  #endif
}

Logger::LogPrinter& Logger::operator()(std::int8_t indent) {
  // Add the indent
  printer_ << getIndent(indent);
  return printer_;
}

std::ostream& Logger::getOS() {
  return outRW_.get();
}

void Logger::setOS(std::ostream& os) {
  outRW_ = os;
}

void Logger::setIndentStr(const std::string& str) {
  indentStr_ = str;
}

std::string Logger::getIndentStr() {
  return indentStr_;
}

void Logger::enable() {
  enabled_ = true;
}

void Logger::disable() {
  enabled_ = false;
}

bool Logger::isEnabled() {
  return enabled_;
}

void Logger::indent(std::int8_t val) {
  indentDepth_ += val;
}

Logger::IndentGuard Logger::indentGuard(std::int8_t val) {
  return IndentGuard(this, val);
}

void Logger::dedent(std::int8_t val) {
  if(indentDepth_ >= val)
    indentDepth_ -= val;
  else
    indentDepth_ = 0;
}

std::string Logger::getIndent(std::int8_t additionalIndent) {
  std::string indent;
  for(auto k = (indentDepth_+additionalIndent); k > 0; --k)
    indent += indentStr_;
  return indent;
}
