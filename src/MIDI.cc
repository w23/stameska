#include "PolledFile.h"
#include "Variables.h"
#include "YamlParser.h"
#include "utils.h"

#include "MIDI.h"

#ifndef __linux__
#error Not supported
#else
#include <alsa/asoundlib.h>
#include <stdio.h>
#endif

#define MSG_ERR(msg, ...) MSG("[ERR] " msg "", ## __VA_ARGS__)

/*
class MIDIDevice : NonCopyable {
public:
	Expected<MIDIDevice, std::string> open(std::string name);

	bool poll(unsigned char *ctl);

private:
	MIDIDevice(std::string name);
	~MIDIDevice();

private:
	const std::string name_;
	const snd_rawmidi_t *handle_;
};
*/

#ifndef MIDI_MAX_DEVICES
#define MIDI_MAX_DEVICES 8
#endif

typedef struct {
	snd_rawmidi_t *handle;
	int errors;
	char name[32];
} AlsaMidiDevice;

static struct {
	AlsaMidiDevice devices[MIDI_MAX_DEVICES];
	int num_devices;
	//int verbose;
} g;


static int midiAddDevice(const char *name) {
	int slot = -1;
	for (int i = 0; i < MIDI_MAX_DEVICES; ++i) {
		if (g.devices[i].handle) {
			if (strncmp(g.devices[i].name, name, sizeof(g.devices[i].name)) == 0)
				return 0;
		} else if (slot < 0)
			slot = i;
	}

	if (slot < 0)
		return 1;

	AlsaMidiDevice *dev = g.devices + slot;
	strncpy(dev->name, name, sizeof(dev->name));
	const int err = snd_rawmidi_open(&dev->handle, NULL, dev->name, SND_RAWMIDI_NONBLOCK);
	if (err < 0)
		return err;

	//MSG("Opened device %s (%s, %s)", dev->name, name, subdevice_name);
	g.num_devices++;
	return 0;
}

int midiOpenAll() {
	//g.verbose = !!getenv("MIDI_VERBOSE");

	int card = -1;
	snd_rawmidi_info_t *info;
	snd_rawmidi_info_alloca(&info);

	for (;;) {
		int err = snd_card_next(&card);
		if (err < 0) {
			MSG_ERR("Failed to get next ALSA card index: %s", snd_strerror(err));
			break;
		}
		if (card < 0)
			break;

		snd_ctl_t *ctl = NULL;
		char card_name[16];
		snprintf(card_name, sizeof(card_name), "hw:%d", card);
		err = snd_ctl_open(&ctl, card_name, 0);
		if (err < 0) {
			MSG_ERR("Failed to open control for card %s: %s", card_name, snd_strerror(err));
			continue;
		}

		int device = -1;
		for (;;) {
			err = snd_ctl_rawmidi_next_device(ctl, &device);
			if (err < 0) {
				MSG_ERR("Failed to get device number for %s: %s", card_name, snd_strerror(err));
				break;
			}
			if (device < 0)
				break;

			snd_rawmidi_info_set_device(info, device);

			snd_rawmidi_info_set_stream(info, SND_RAWMIDI_STREAM_INPUT);
			err = snd_ctl_rawmidi_info(ctl, info);
			const int inputs = err < 0 ? 0 : snd_rawmidi_info_get_subdevices_count(info);
			for (int i = 0; i < inputs; ++i) {
				snd_rawmidi_info_set_subdevice(info, i);
				err = snd_ctl_rawmidi_info(ctl, info);
				if (err < 0) {
					MSG_ERR("Failed to get rawmidi info on device hw:%d,%d,%d: %s", card, device, i, snd_strerror(err));
					continue;
				}
				const char *name = snd_rawmidi_info_get_name(info);
				const char *subdevice_name = snd_rawmidi_info_get_subdevice_name(info);

				if (g.num_devices == MIDI_MAX_DEVICES) {
					MSG_ERR("Cannot add device hw:%d,%d,%d (%s, %s): maximum number of MIDI devices (%d) reached",
						card, device, i, name, subdevice_name, MIDI_MAX_DEVICES);
					continue;
				}

				char sname[32];
				snprintf(sname, sizeof(sname), "hw:%d,%d,%d", card, device, i);
				err = midiAddDevice(sname);
				if (err < 0) {
					MSG_ERR("Failed to open device %s (%s, %s): %s", sname, name, subdevice_name, snd_strerror(err));
					continue;
				}
			}
		}
		snd_ctl_close(ctl);
	}

	return g.num_devices;
}

void midiClose() {
	for (int i = 0; i < g.num_devices; ++i) {
		AlsaMidiDevice *dev = g.devices + i;
		if (dev->handle) {
			snd_rawmidi_close(dev->handle);
			dev->handle = NULL;
		}
	}
}

static unsigned char midi_ctls[256];

int midiPoll() {
	for (int d = 0; d < MIDI_MAX_DEVICES; ++d) {
		AlsaMidiDevice *dev = g.devices + d;
		if (!dev->handle)
			continue;
		unsigned char midi_buf[1024];
		const int err = snd_rawmidi_read(dev->handle, midi_buf, sizeof(midi_buf));
		if (err < 0 && err != -EAGAIN) {
			MSG_ERR("Failed to read stream from device %s: %s", dev->name, snd_strerror(err));
			++dev->errors;
			if (dev->errors == 10) {
				MSG_ERR("Failed to read device %s 10 times in a row, disabling", dev->name);
				snd_rawmidi_close(dev->handle);
				dev->handle = NULL;
				--g.num_devices;
			}
			continue;
		}

		dev->errors = 0;

		if (err <= 0)
			continue;

		for (int i = 0; i < err; ++i) {
			const unsigned char b = midi_buf[i];
			if (!(b & 0x80))
				continue; /* skip unsync data bytes */

			if (err - i < 3)
				break; /* expect at least 2 args */

			const int channel = b & 0x0f;
			if ((b & 0xf0) == 0xb0) { /* control change */
				const int controller = midi_buf[i + 1];
				const int value = midi_buf[i + 2];
				//if (g.verbose)
					MSG("%s: control change ch=%d p=%d v=%d", dev->name, channel, controller, value);
				midi_ctls[controller] = value;
				i += 2;
			}
		}
	}

	return 0;
}

class MIDIScope : public IScope {
public:
	MIDIScope(std::string_view filename)
		: file_(filename)
	{
	}

	bool poll(unsigned int poll_seq) {
		if (!file_.poll(poll_seq))
			return false;

		const auto result = yaml::parse(file_.string());
		if (!result) {
			MSG("Error parsing constants: %s", result.error().c_str());
			return false;
		}

		const auto mapping_result = result.value().getMapping();
		if (!mapping_result) {
			MSG("Error getting toplevel yaml mapping: %s", mapping_result.error().c_str());
			return false;
		}

		const yaml::Mapping &map = mapping_result.value();

		std::map<std::string, int> newmap;
		for (auto const &[name, value]: map.map()) {
			const auto expect_int = value.getInt();
			if (!expect_int) {
				MSG("Error parsing mapping: %s", expect_int.error().c_str());
				return false;
			}

			const long int index = expect_int.value();
			if (index < 0 || index > 127) {
				MSG("Index for %s is %ld and out of [0, 127] bounds",name.c_str(), index);
				return false;
			}

			newmap[name] = (int)index;
		}

		map_ = std::move(newmap);

		return true;
	}

	virtual Value getValue(const std::string& name, int comps) override {
		(void)(comps);
		const auto &it = map_.find(name);
		if (it == map_.end())
			return Value{0,0,0,0};

		const float f = midi_ctls[it->second] / 127.f;
		return Value{f,f,f,f};
	}

private:
	PolledFile file_;
	std::map<std::string, int> map_;
};

static std::unique_ptr<MIDIScope> scope;

void MIDI::init(std::string_view filename) {
	scope.reset(new MIDIScope(filename));
	midiOpenAll();
}

bool MIDI::active() { return !!scope; }

void MIDI::poll() {
	static unsigned int poll_seq = 0;
	scope->poll(poll_seq++);
	if (!midiPoll())
		midiOpenAll();
}

IScope &MIDI::getScope() { return *scope.get(); }
