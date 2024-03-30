#include "input_neuron.hpp"
#include "functions.hpp"
#include "message.hpp"
#include "neuron.hpp"

InputNeuron::InputNeuron(int _id, NeuronGroup *group)
    : Neuron(_id, -1, group, Input) {
  this->activate();
}

void InputNeuron::run_in_group() {

  if (!this->check_refractory_period()) {
    lg.log_group_neuron_state(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring input",
        this->get_group()->get_id(), this->get_id());
    return;
  }

  if (poisson_result()) {

    double time = lg.get_time_stamp();

    this->update_potential(this->input_value);

    lg.add_data(this->get_group()->get_id(), this->get_id(),
                this->membrane_potential, time, this->get_type(), Stimulus);

    lg.log_group_neuron_value(
        INFO,
        "(Input) (%d) Neuron %d poisson success! Adding input value to "
        "membrane_potential. membrane_potential now %f",
        this->get_group()->get_id(), this->get_id(), this->membrane_potential);
  }

  if (this->membrane_potential >= ACTIVATION_THRESHOLD) {
    this->send_messages_in_group();
  }
}

bool InputNeuron::poisson_result() {

  double roll = (double)rand() / RAND_MAX;

  if (roll <= this->probalility_of_success) {
    return true;
  } else {
    return false;
  }
}

bool InputNeuron::check_refractory_period() {

  double timestamp = lg.get_time_stamp();

  // #askpedram

  // first check refractory status
  if (timestamp < this->refractory_start + REFRACTORY_DURATION) {
    lg.add_data(this->group->get_id(), this->id, this->membrane_potential,
                timestamp, this->get_type(), Stimulus);
    lg.log_group_neuron_state(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring message",
        this->get_group()->get_id(), this->get_id());
    return false;
  }

  return true;
}

void InputNeuron::send_messages_in_group() {

  // loop through all neighbors
  for (const auto &pair : this->_postsynaptic) {

    lg.log_group_neuron_interaction(INFO,
                                    "(Input) Group %d: Neuron %d is sending a "
                                    "mesage to Group %d: Neuron %d",
                                    this->group->get_id(), this->id,
                                    pair.first->get_group()->get_id(),
                                    pair.first->get_id());

    // construct message
    Message *message = new Message;
    message->target_neuron_group = pair.first->get_group();
    message->post_synaptic_neuron = pair.first;
    message->timestamp = lg.get_time_stamp();
    message->message_type = From_Neighbor;

    // calculate message
    pthread_mutex_lock(&potential_mutex);
    message->message = this->membrane_potential *
                       this->_postsynaptic[pair.first] *
                       this->excit_inhib_value;
    pthread_mutex_unlock(&potential_mutex);

    lg.log_group_neuron_value(
        DEBUG2, "Accumulated for (Input) Group %d: Neuron %d is %f",
        this->group->get_id(), this->id, this->membrane_potential);

    lg.log_group_neuron_interaction(
        DEBUG2,
        "Weight for (Input) Group %d: Neuron %d to Group %d: Neuron %d is %f",
        this->get_group()->get_id(), this->get_id(),
        pair.first->get_group()->get_id(), pair.first->get_id(), pair.second);

    lg.log_group_neuron_value(DEBUG2, "Group %d: Neuron %d modifier is %d",
                              this->group->get_id(), this->get_id(),
                              this->excit_inhib_value);

    lg.log_group_neuron_interaction(
        INFO, "Message from  (Input) (%d) Neuron %d to (%d) Neuron %d is %f",
        this->group->get_id(), this->id, pair.first->get_group()->get_id(),
        pair.first->get_id(), message->message);

    // activate neighbor
    pair.first->activate();

    // add message to target
    message->post_synaptic_neuron->add_message(message);
  }

  lg.log_group_neuron_state(
      INFO,
      "(%d) Neuron %d reached activation threshold, entering refractory phase",
      this->group->get_id(), this->id);

  this->refractory();
}

void InputNeuron::set_input_value(double value) {
  this->input_value = value;
  lg.log_group_neuron_value(DEBUG3,
                            "(Input) (%d) Neuron %d input value set to %f",
                            this->get_group()->get_id(), this->get_id(), value);
}
