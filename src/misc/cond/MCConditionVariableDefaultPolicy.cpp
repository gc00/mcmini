#include "mcmini/misc/cond/MCConditionVariableDefaultPolicy.hpp"
#include <algorithm>
#include <memory>

namespace mcmini {

void
ConditionVariableDefaultPolicy::receive_broadcast_message()
{
  // Move everyone into the get-out-of-jail free place
  // from the wake group list
  for (const WakeGroup &wg : this->wake_groups) {
    for (const tid_t signaled_thread : wg) {
      this->broadcast_eligible_threads.insert(signaled_thread);
    }
  }
  this->wake_groups.clear();
}

bool
ConditionVariableDefaultPolicy::thread_can_exit(tid_t tid) const
{
  // Either you're eligible to wake up because:

  // 1. You were around during a broadcast message
  if (this->broadcast_eligible_threads.count(tid) > 0) {
    return true;
  }

  // 2. OR you can now consume a signal
  return std::any_of(
    wake_groups.begin(), wake_groups.end(),
    [=](const WakeGroup &wg) { return wg.contains(tid); });
}

void
ConditionVariableDefaultPolicy::wake_thread(tid_t tid)
{
  // To correctly match the semantics of condition variables,
  // if a thread was present in the condition variable during a
  // broadcast, we DO NOT want it to consume signals which arrive
  // later, since we want to treat the thread as no longer waiting
  // on the condition variable
  if (this->broadcast_eligible_threads.count(tid) > 0) {
    this->broadcast_eligible_threads.erase(tid);
  } else {
    // Otherwise, we should consume the FIRST signal
    // we're located in: we want to ensure we allow
    // the most possible threads to
    const auto signal_to_consume = std::find_if(
      wake_groups.begin(), wake_groups.end(),
      [=](const WakeGroup &wg) { return wg.contains(tid); });

    // If `signal_to_consume == wake_groups.end()`, we are attempting
    // to wake a thread which can neither consume a signal nor has
    // been woken from a prior broadcast.
    MC_ASSERT(signal_to_consume != wake_groups.end());
    wake_groups.erase(signal_to_consume);
  }

  // Additionally, remove this thread from any other
  // groups so it doesn't attempt to consume a signal
  // in the future
  for (WakeGroup &wg : this->wake_groups) {
    wg.remove_candidate_thread(tid);
  }
}

} // namespace mcmini