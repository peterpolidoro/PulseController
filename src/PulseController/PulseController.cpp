// ----------------------------------------------------------------------------
// PulseController.cpp
//
//
// Authors:
// Peter Polidoro polidorop@janelia.hhmi.org
// ----------------------------------------------------------------------------
#include "../PulseController.h"


using namespace pulse_controller;

PulseController::PulseController()
{
}

void PulseController::setup()
{
  // Parent Setup
  PowerSwitchController::setup();

  // Reset Watchdog
  resetWatchdog();

  // Variable Setup

  // Set Device ID
  modular_server_.setDeviceName(constants::device_name);

  // Add Hardware

  // Interrupts
  modular_server::Interrupt & bnc_a_interrupt = modular_server_.interrupt(modular_device_base::constants::bnc_a_interrupt_name);
  modular_server::Interrupt & btn_a_interrupt = modular_server_.interrupt(modular_device_base::constants::btn_a_interrupt_name);

#if defined(__MK64FX512__)
  modular_server::Interrupt & bnc_b_interrupt = modular_server_.interrupt(modular_device_base::constants::bnc_b_interrupt_name);
  modular_server::Interrupt & btn_b_interrupt = modular_server_.interrupt(modular_device_base::constants::btn_b_interrupt_name);
#endif

  // Add Firmware
  modular_server_.addFirmware(constants::firmware_info,
                              properties_,
                              parameters_,
                              functions_,
                              callbacks_);

  // Properties
  modular_server::Property & pulse_channels_property = modular_server_.createProperty(constants::pulse_channels_property_name,constants::pulse_channels_default);
  pulse_channels_property.setArrayLengthDefault(constants::pulse_channels_array_length_default);
  pulse_channels_property.setRange((long)0,(long)power_switch_controller::constants::CHANNEL_COUNT-1);

  modular_server::Property & pulse_delay_property = modular_server_.createProperty(constants::pulse_delay_property_name,constants::pulse_delay_default);
  pulse_delay_property.setUnits(power_switch_controller::constants::ms_units);
  pulse_delay_property.setRange(power_switch_controller::constants::delay_min,power_switch_controller::constants::delay_max);

  modular_server::Property & pulse_period_property = modular_server_.createProperty(constants::pulse_period_property_name,constants::pulse_period_default);
  pulse_period_property.setUnits(power_switch_controller::constants::ms_units);
  pulse_period_property.setRange(power_switch_controller::constants::period_min,power_switch_controller::constants::period_max);

  modular_server::Property & pulse_on_duration_property = modular_server_.createProperty(constants::pulse_on_duration_property_name,constants::pulse_on_duration_default);
  pulse_on_duration_property.setUnits(power_switch_controller::constants::ms_units);
  pulse_on_duration_property.setRange(power_switch_controller::constants::on_duration_min,power_switch_controller::constants::on_duration_max);

  modular_server::Property & pulse_count_property = modular_server_.createProperty(constants::pulse_count_property_name,constants::pulse_count_default);
  pulse_count_property.setUnits(power_switch_controller::constants::ms_units);
  pulse_count_property.setRange(power_switch_controller::constants::count_min,power_switch_controller::constants::count_max);

  // Parameters

  // Functions

  // Callbacks
  modular_server::Callback & pulse_callback = modular_server_.createCallback(constants::pulse_callback_name);
  pulse_callback.attachFunctor(makeFunctor((Functor1<modular_server::Interrupt *> *)0,*this,&PulseController::pulseHandler));
  pulse_callback.attachTo(bnc_a_interrupt,modular_server::interrupt::mode_falling);
  pulse_callback.attachTo(btn_a_interrupt,modular_server::interrupt::mode_falling);

}

void PulseController::pulse()
{
  modular_server::Property & pulse_channels_property = modular_server_.property(constants::pulse_channels_property_name);
  size_t array_length = pulse_channels_property.getArrayLength();
  long pulse_channels[array_length];
  pulse_channels_property.getValue(pulse_channels,array_length);
  uint32_t channels = 0;
  uint32_t bit = 1;
  for (size_t i=0; i<array_length; ++i)
  {
    long channel = pulse_channels[i];
    channels |= bit << channel;
  }

  long delay;
  modular_server_.property(constants::pulse_delay_property_name).getValue(delay);

  long period;
  modular_server_.property(constants::pulse_period_property_name).getValue(period);

  long on_duration;
  modular_server_.property(constants::pulse_on_duration_property_name).getValue(on_duration);

  long count;
  modular_server_.property(constants::pulse_count_property_name).getValue(count);

  addPwm(channels,
         delay,
         period,
         on_duration,
         count);
}

// Handlers must be non-blocking (avoid 'delay')
//
// modular_server_.parameter(parameter_name).getValue(value) value type must be either:
// fixed-point number (int, long, etc.)
// floating-point number (float, double)
// bool
// const char *
// ArduinoJson::JsonArray *
// ArduinoJson::JsonObject *
//
// For more info read about ArduinoJson parsing https://github.com/janelia-arduino/ArduinoJson
//
// modular_server_.property(property_name).getValue(value) value type must match the property default type
// modular_server_.property(property_name).setValue(value) value type must match the property default type
// modular_server_.property(property_name).getElementValue(element_index,value) value type must match the property array element default type
// modular_server_.property(property_name).setElementValue(element_index,value) value type must match the property array element default type

void PulseController::pulseHandler(modular_server::Interrupt * interrupt_ptr)
{
  pulse();
}
