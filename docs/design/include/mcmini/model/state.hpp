#pragma once

#include <memory>
#include <unordered_map>

#include "mcmini/forwards.hpp"
#include "mcmini/model/visible_object.hpp"

namespace mcmini::model {
/**
 * @brief A particular snapshot in time of a program undergoing verification
 * from the perspective of the verifier.
 *
 * A `state` is simply a collection of visible objects. Each
 */
class state {
 public:
  /**
   * @brief Determines whether this state contains the object with id _id_.
   *
   * @returns true if there is an object associated with id _id_ in this state,
   * and false otherwise.
   */
  virtual bool contains_object_with_id(visible_object::objid_t id) const = 0;

  /**
   * @brief Begin tracking a new visible object with initial state
   * _initial_state_.
   *
   * A state
   *
   * @param initial_state the initial state of the new object
   * @return the new id that is assigned to the newly created object. This id is
   * unique from every other id assigned to the objects in this state.
   */
  virtual visible_object::objid_t track_new_visible_object(
      std::unique_ptr<visible_object_state> initial_state) = 0;

  /**
   *
   */
  virtual void record_new_state_for_visible_object(
      visible_object::objid_t, std::unique_ptr<visible_object_state>) = 0;

  /**
   * @brief Retrieve the object with id _id_ in this state.
   */
  virtual const visible_object_state &get_state_of_object(
      visible_object::objid_t id) const = 0;

  /**
   * @brief Creates a copy of the given state.
   *
   * All visible objects in the underlying state are copied into the new state
   * and are independently modifiable with respect to the first state.
   */
  virtual std::unique_ptr<state> clone() const = 0;
};

// class detached_state : public state {
//   std::unordered_map<visible_object::objid_t,
//                      std::unique_ptr<const visible_object_state>>
//       visible_object_states_to_ids;
// };

}  // namespace mcmini::model