// //
// // The task class is a base class for acquiring samples.
// //
// // Derived class are adctask and fallingedgetask.
// //
// // The role of a task is to abstract away the details of acquiring samples for filters.
// // The filters accept and process the samples without worrying how to actually generate the data.
// //

// #pragma once

// #include <Arduino.h>

// #include "filter.h"

// class task
// {

//  public:

//  task() = default;
 
//   // Deleting the task will also delete the attached filter.
//   ~task();

//   // Service the task.
//   virtual void service(uint8_t sample_value) = 0;

//  protected:

//   task(filter* sample_filter);

//   // The sample filter associated with the task.
//   filter* filter_ = nullptr;

//   // This will be true if the task has been started and running.
//   // Derived classes should extend this method to start up whatever resources they need.
//   bool started_ = false;

// };
