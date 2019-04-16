//----------------------------------------------------------------------------//
// Part of the Fox project, licensed under the MIT license.
// See LICENSE.txt in the project root for license information.     
// File : BCGen.cpp                      
// Author : Pierre van Houtryve                
//----------------------------------------------------------------------------//

#include "Fox/BCGen/BCGen.hpp"
#include "Fox/AST/ASTContext.hpp"
#include "Fox/BC/BCModule.hpp"
#include <string>

using namespace fox;

BCGen::BCGen(ASTContext& ctxt, BCModule& theModule) : 
  ctxt(ctxt), diagEngine(ctxt.diagEngine), theModule(theModule) {}

constant_id_t BCGen::getConstantID(string_view strview) {
  // Check if it exists in the map
  auto& map = strConstsMap_;
  std::size_t hash = std::hash<string_view>()(strview);
  auto it = map.find(hash);
  // Found it in the map: return
  if (it != map.end()) return it->second;
  // Else insert it
  std::size_t rawID = theModule.addStringConstant(strview);
  // TODO: Replace this by a proper diagnostic
  assert((rawID < bc_limits::max_constant_id)
    && "cannot insert constant: limit of constant_id_t reached");
  // Insert it in the map
  auto kID = static_cast<constant_id_t>(rawID);
  map.insert({hash, kID});
  return kID;
}

constant_id_t BCGen::getConstantID(FoxInt value) {
  // Check if it exists in the map
  auto& map = intConstsMap_;
  auto it = map.find(value);
  // Found it in the map: return
  if (it != map.end()) return it->second;
  // Else insert it
  std::size_t rawID = theModule.addIntConstant(value);
  // TODO: Replace this by a proper diagnostic
  assert((rawID < bc_limits::max_constant_id)
    && "cannot insert constant: limit of constant_id_t reached");
  // Insert it in the map
  auto kID = static_cast<constant_id_t>(rawID);
  map.insert({value, kID});
  return kID;
}

constant_id_t BCGen::getConstantID(FoxDouble value) {
  // Check if it exists in the map
  auto& map = doubleConstsMap_;
  auto it = map.find(value);
  // Found it in the map: return
  if (it != map.end()) return it->second;
  // Else insert it
  std::size_t rawID = theModule.addDoubleConstant(value);
  // TODO: Replace this by a proper diagnostic
  assert((rawID < bc_limits::max_constant_id)
    && "cannot insert constant: limit of constant_id_t reached");
  // Insert it in the map
  auto kID = static_cast<constant_id_t>(rawID);
  map.insert({value, kID});
  return kID;
}
