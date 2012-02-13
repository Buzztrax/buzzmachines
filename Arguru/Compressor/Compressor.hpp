#ifndef ARGURU_COMPRESSOR_HPP
#define ARGURU_COMPRESSOR_HPP

#include <cstdio>

#include <zzub/signature.h>
#include <zzub/plugin.h>

struct Gvals {
  unsigned short paraGain : 8;
  unsigned short paraThreshold : 8;
  unsigned short paraRatio : 8;
  unsigned short paraAttack : 8;
  unsigned short paraRelease : 8;
  unsigned short paraClip : 8;
} __attribute__((__packed__));

const zzub::parameter *paraGain = 0;
const zzub::parameter *paraThreshold = 0;
const zzub::parameter *paraRatio = 0;
const zzub::parameter *paraAttack = 0;
const zzub::parameter *paraRelease = 0;
const zzub::parameter *paraClip = 0;

enum { 
  paramGain = 0, 
  paramThreshold, 
  paramRatio, 
  paramAttack, 
  paramRelease, 
  paramClip 
};

float linear_to_deci_bell(float value) {
  return 10.0 * log10(value);
}

float deci_bell_to_linear(float db) {
  return pow(10, db / 10.0);
}

const char *zzub_get_signature() { 
  return ZZUB_SIGNATURE; 
}

class Compressor : public zzub::plugin {
private:
  Gvals gval;
  float currentGain;
  unsigned int currentSR;
  int Vals[6];
public:
  Compressor();
  virtual ~Compressor();
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

struct CompressorInfo : zzub::info {
  CompressorInfo() {
    this->flags = 
      zzub::plugin_flag_has_audio_input | zzub::plugin_flag_has_audio_output;
    this->name = "Arguru Compressor";
    this->short_name = "Compressor";
    this->author = "Arguru";
    this->uri = "@libneil/arguru/effect/compressor";
    paraGain = &add_global_parameter()
      .set_byte()
      .set_name("Input Gain")
      .set_description("Input Gain")
      .set_value_min(0x00)
      .set_value_max(0x80)
      .set_value_none(0xFF)
      .set_state_flag()
      .set_value_default(0x00);
    paraThreshold = &add_global_parameter()
      .set_byte()
      .set_name("Threshold")
      .set_description("Threshold")
      .set_value_min(0x00)
      .set_value_max(0x80)
      .set_value_none(0xFF)
      .set_state_flag()
      .set_value_default(0x40);
    paraRatio = &add_global_parameter()
      .set_byte()
      .set_name("Ratio")
      .set_description("Ratio")
      .set_value_min(0x00)
      .set_value_max(0x10)
      .set_value_none(0xFF)
      .set_state_flag()
      .set_value_default(0x10);
    paraAttack = &add_global_parameter()
      .set_byte()
      .set_name("Attack")
      .set_description("Attack")
      .set_value_min(0x00)
      .set_value_max(0x80)
      .set_value_none(0xFF)
      .set_state_flag()
      .set_value_default(0x06);
    paraRelease = &add_global_parameter()
      .set_byte()
      .set_name("Release")
      .set_description("Release")
      .set_value_min(0x00)
      .set_value_max(0x80)
      .set_value_none(0xFF)
      .set_state_flag()
      .set_value_default(0x2C);
    paraClip = &add_global_parameter()
      .set_byte()
      .set_name("Soft clip")
      .set_description("Soft clip")
      .set_value_min(0x00)
      .set_value_max(0x01)
      .set_value_none(0xFF)
      .set_state_flag()
      .set_value_default(0x00);

  }
  virtual zzub::plugin* create_plugin() const { return new Compressor(); }
  virtual bool store_info(zzub::archive *data) const { return false; }
} MachineInfo;

struct Arguru_Compressor_PluginCollection : zzub::plugincollection {
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
  return new Arguru_Compressor_PluginCollection();
}

#endif // ARGURU_COMPRESSOR_HPP
