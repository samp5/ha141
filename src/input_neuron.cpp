#include "input_neuron.hpp"
#include "functions.hpp"
#include "message.hpp"
#include "neuron.hpp"
#include <pthread.h>

void InputNeuron::run_in_group() {

  if (::switching_stimulus) {
    // wait for all input neurons to switch to the new stimulus
    pthread_mutex_lock(&::stimulus_switch_mutex);
    while (::switching_stimulus) {
      pthread_cond_wait(&::stimulus_switch_cond, &::stimulus_switch_mutex);
    }
    pthread_mutex_unlock(&::stimulus_switch_mutex);
  }

  if (!this->check_refractory_period()) {
    lg.log_group_neuron_state(
        INFO,
        "INPUT: (%d) Neuron %d is still in refractory period, ignoring input",
        this->get_group()->get_id(), this->get_id());
    return;
  }

  double time = lg.get_time_stamp();
  this->retroactive_decay(this->last_decay, time);

  if (this->membrane_potential >= ACTIVATION_THRESHOLD) {
    this->send_messages_in_group();
  }

  if (poisson_result()) {

    double time_rn = lg.get_time_stamp();

    this->update_potential(this->input_value);

    lg.add_data(this->get_group()->get_id(), this->get_id(),
                this->membrane_potential, time_rn, this->get_type(), Stimulus,
                this);

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

    lg.log_group_neuron_state(
        INFO, "(%d) Neuron %d is still in refractory period, ignoring message",
        this->get_group()->get_id(), this->get_id());

    return false;
  }

  return true;
}

void InputNeuron::send_messages_in_group() {

  for (const auto synapse : getSynapses()) {
    synapse->propagate();
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

void InputNeuron::reset() {
  this->membrane_potential = INITIAL_MEMBRANE_POTENTIAL;
  this->last_decay = lg.get_time_stamp();
  this->refractory_start = 0;
}
