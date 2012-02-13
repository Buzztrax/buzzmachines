#ifndef ARGURU_DISTORTION_HPP
#define ARGURU_DISTORTION_HPP

#include <cstdio>
#include <stdint.h>

#include <zzub/signature.h>
#include <zzub/plugin.h>

struct Gvals {
  uint16_t paraPreGain;
  uint16_t paraThresholdNeg;
  uint16_t paraThreshold;
  uint16_t paraGain;
  uint8_t paraInvert;
  uint8_t paraMode;
} __attribute__((__packed__));

const zzub::parameter *paraPreGain = 0;
const zzub::parameter *paraThresholdNeg = 0;
const zzub::parameter *paraThreshold = 0;
const zzub::parameter *paraGain = 0;
const zzub::parameter *paraInvert = 0;
const zzub::parameter *paraMode = 0;

float linear_to_deci_bell(float value) {
  return 10.0 * log10(value);
}

float deci_bell_to_linear(float db) {
  return pow(10, db / 10.0);
}

const char *zzub_get_signature() { 
  return ZZUB_SIGNATURE; 
}

class Distortion : public zzub::plugin {
private:
  Gvals gval;
  inline void Clip(float *psamplesleft, float const threshold, 
		   float const negthreshold, float const wet);
  inline void Saturate(float *psamplesleft, float const threshold,
		       float const negthreshold, float const wet);
  float leftLim;
  float rightLim;
  int Vals[6];
public:
  Distortion();
  virtual ~Distortion();
  virtual void init(zzub::archive* pi);
  virtual void process_events();
  virtual bool process_stereo(float **pin, float **pout, 
			      int numsamples, int mode);
  virtual bool process_offline(float **pin, float **pout, 
			       int *numsamples, int *channels, 
			       int *samplerate) { return false; }
  virtual const char * describe_value(int param, int value); 
  virtual void process_controller_events() {}
  virtual void destroy();
  virtual void stop() {}
  virtual void load(zzub::archive *arc) {}
  virtual void save(zzub::archive*) {}
  virtual void attributes_changed() {}
  virtual void command(int) {}
  virtual void set_track_count(int) {}
  virtual void mute_track(int) {}
  virtual bool is_track_muted(int) const { return false; }
  virtual void midi_note(int, int, int) {}
  virtual void event(unsigned int) {}
  virtual const zzub::envelope_info** get_envelope_infos() { return 0; }
  virtual bool play_wave(int, int, float) { return false; }
  virtual void stop_wave() {}
  virtual int get_wave_envelope_play_position(int) { return -1; }
  virtual const char* describe_param(int) { return 0; }
  virtual bool set_instrument(const char*) { return false; }
  virtual void get_sub_menu(int, zzub::outstream*) {}
  virtual void add_input(const char*, zzub::connection_type) {}
  virtual void delete_input(const char*, zzub::connection_type) {}
  virtual void rename_input(const char*, const char*) {}
  virtual void input(float**, int, float) {}
  virtual void midi_control_change(int, int, int) {}
  virtual bool handle_input(int, int, int) { return false; }
  virtual void process_midi_events(zzub::midi_message* pin, int nummessages) {}
  virtual void get_midi_output_names(zzub::outstream *pout) {}
  virtual void set_stream_source(const char* resource) {}
  virtual const char* get_stream_source() { return 0; }
  virtual void play_pattern(int index) {}
  virtual void configure(const char *key, const char *value) {}
};

struct DistortionInfo : zzub::info {
  DistortionInfo() {
    this->flags = 
      zzub::plugin_flag_has_audio_input | zzub::plugin_flag_has_audio_output;
    this->name = "Arguru Distortion";
    this->short_name = "Distortion";
    this->author = "Arguru";
    this->uri = "@libneil/arguru/effect/distortion";
    paraPreGain = &add_global_parameter()
      .set_word()
      .set_name("Input Gain")
      .set_description("Input Gain")
      .set_value_min(0x0001)
      .set_value_max(0x0800)
      .set_value_none(0xFFFF)
      .set_state_flag()
      .set_value_default(0x0100);
    paraThresholdNeg = &add_global_parameter()
      .set_word()
      .set_name("Threshold (-)")
      .set_description("Threshold level (negative)")
      .set_value_min(0x0001)
      .set_value_max(0x8000)
      .set_value_none(0xFFFF)
      .set_state_flag()
      .set_value_default(0x200);
    paraThreshold = &add_global_parameter()
      .set_word()
      .set_name("Threshold (+)")
      .set_description("Threshold level (positive)")
      .set_value_min(0x0001)
      .set_value_max(0x8000)
      .set_value_none(0xFFFF)
      .set_state_flag()
      .set_value_default(0x200);
    paraGain = &add_global_parameter()
      .set_word()
      .set_name("Output Gain")
      .set_description("Output Gain")
      .set_value_min(0x0001)
      .set_value_max(0x0800)
      .set_value_none(0xFFFF)
      .set_state_flag()
      .set_value_default(0x0400);
    paraInvert = &add_global_parameter()
      .set_byte()
      .set_name("Phase inversor")
      .set_description("Stereo phase inversor")
      .set_value_min(0x00)
      .set_value_max(0x01)
      .set_value_none(0xFF)
      .set_state_flag()
      .set_value_default(0x00);
    paraMode = &add_global_parameter()
      .set_byte()
      .set_name("Mode")
      .set_description("Operational mode")
      .set_value_min(0x00)
      .set_value_max(0x01)
      .set_value_none(0xFF)
      .set_state_flag()
      .set_value_default(0x00);
  }
  virtual zzub::plugin* create_plugin() const { return new Distortion(); }
  virtual bool store_info(zzub::archive *data) const { return false; }
} MachineInfo;

struct Arguru_Distortion_PluginCollection : zzub::plugincollection {
  virtual void initialize(zzub::pluginfactory *factory) {
    factory->register_info(&MachineInfo);
  }
  virtual const zzub::info *get_info(const char *uri, zzub::archive *data) { 
    return 0; 
  }
  virtual void destroy() { 
    delete this; 
  }
  virtual const char *get_uri() { 
    return 0;
  }
  virtual void configure(const char *key, const char *value) {

  }
};

zzub::plugincollection *zzub_get_plugincollection() {
  return new Arguru_Distortion_PluginCollection();
}

#endif // ARGURU_DISTORTION_HPP
