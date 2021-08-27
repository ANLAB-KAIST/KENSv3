/*
 * E_Module.cpp
 *
 *  Created on: 2014. 11. 1.
 *      Author: Keunhong Lee
 */

#include <E/E_Module.hpp>
#include <E/E_System.hpp>

#ifdef HAVE_DEMANGLE
#include <cxxabi.h>
#endif

namespace E {

Module::Module(System &system) : system(system), id(0) {}

Module::~Module() {}

bool Module::cancelMessage(UUID timer) {
  return this->system.cancelMessage(timer);
}

UUID Module::sendMessage(const ModuleID to, Module::Message message,
                         Time timeAfter) {
  return system.sendMessage(id, to, std::move(message), timeAfter);
}

UUID Module::sendMessageSelf(Module::Message message, Time timeAfter) {
  return sendMessage(id, std::move(message), timeAfter);
}

std::string Module::getModuleName() {

  const char *type_name = typeid(*this).name();
#ifdef HAVE_DEMANGLE
  auto ptr = std::unique_ptr<char, decltype(&std::free)>{
      abi::__cxa_demangle(type_name, nullptr, nullptr, nullptr), std::free};
  const char *demangled_name = ptr.get();
#else
  const char *demangled_name = type_name;
#endif

  return {demangled_name};
}

std::string Module::getModuleName(const ModuleID moduleID) {
  return system.getModuleName(moduleID);
}
Time Module::getCurrentTime() { return system.getCurrentTime(); }

Module::EmptyMessage &Module::EmptyMessage::shared() {
  static EmptyMessage __inner;
  return __inner;
}

} // namespace E
