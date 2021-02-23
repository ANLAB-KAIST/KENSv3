/*
 * E_Processor.cpp
 *
 *  Created on: 2014. 12. 3.
 *      Author: Keunhong Lee
 */

#include <E/E_System.hpp>
#include <E/Scheduling/E_Computer.hpp>
#include <E/Scheduling/E_Job.hpp>
#include <E/Scheduling/E_Processor.hpp>

namespace E {

Processor::Processor(Computer *computer, CPUID id, Time overhead)
    : Module(computer->getSystem()) {
  this->id = id;
  this->currentJob = nullptr;
  this->lastExecuted = 0;
  this->computer = computer;
  this->currentRunID = 0;
  this->isRunning = false;
  this->overhead = overhead;
}
Processor::~Processor() {}

Job *Processor::getCurrentJob() { return this->currentJob; }
CPUID Processor::getID() { return this->id; }

Time Processor::getResource() {
  Time current = this->getSystem()->getCurrentTime();
  print_log(Log::DEBUG, "Process wake up at %" PRIu64 ", prev is %" PRIu64 "",
            current, this->lastExecuted);
  assert(current >= this->lastExecuted);
  Time run = current - this->lastExecuted;
  this->lastExecuted = current;

  return run;
}

void Processor::deleteJob() {
  assert(this->currentJob != nullptr);
  Time run = this->getResource();
  if (run > 0) {
    if (run < overhead)
      run = 0;
    else
      run -= overhead;
  }
  this->currentJob->execute(run);

  if (this->isRunning) {
    print_log(Log::DEBUG, "Message canceled, %" PRIu64 "", this->lastExecuted);
    this->cancelMessage(this->currentRunID);
    this->isRunning = false;
  }
  this->currentJob->processor = nullptr;

  this->currentJob = nullptr;
  this->currentRunID = 0;
}

void Processor::assignJob(Job *job) {
  assert(this->currentJob == nullptr);
  assert(job->processor == nullptr);
  this->lastExecuted = this->getSystem()->getCurrentTime();
  this->currentJob = job;
  job->processor = this;
  Time remaining = this->currentJob->getRemaining();
  this->getResource();
  assert(isRunning == false);
  // if(remaining > 0)
  {
    Computer::Message *comMessage = new Computer::Message;
    comMessage->type = Computer::MessageType::JOB_RUN;
    comMessage->runningJob = this->currentJob;
    if (remaining != 0)
      remaining += overhead;
    this->currentRunID = this->sendMessage(computer, comMessage, remaining);
    print_log(Log::DEBUG,
              "Set execution at %" PRIu64 " + %" PRIu64 " = %" PRIu64
              ", im %" PRIu64 "",
              this->getSystem()->getCurrentTime(), remaining,
              this->getSystem()->getCurrentTime() + remaining,
              this->lastExecuted);
    this->isRunning = true;
  }
}

Module::Message *Processor::messageReceived(Module *from,
                                            Module::Message *message) {
  (void)from;
  (void)message;
  assert(0);
  return nullptr;
}
void Processor::messageFinished(Module *to, Module::Message *message,
                                Module::Message *response) {
  (void)to;
  (void)response;
  delete message;
}
void Processor::messageCancelled(Module *to, Module::Message *message) {
  (void)to;
  delete message;
}

} // namespace E
