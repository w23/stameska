#pragma once

#include "Variables.h"
#include "rocket/lib/sync.h"
#include <functional>
#include <map>
#include <memory>
#include <string>

class Rocket : public IAutomation {
public:
	typedef std::function<void(int)> PauseCallback;
	typedef std::function<void(int)> SetRowCallback;
	typedef std::function<int()> IsPlayingCallback;

	Rocket(PauseCallback &&pause, SetRowCallback &&setRow, IsPlayingCallback &&isPlaying);
	virtual ~Rocket();

	virtual void update(float row) override;
	virtual Value getValue(const std::string& name, int comps) override;

	virtual void save() const override;
	virtual Expected<ExportResult, std::string> writeExport(std::string_view config, const shader::UniformsMap &uniforms) const override;

private:
	static void pause(void *, int);
	static void set_row(void *, int);
	static int is_playing(void *);

private:
	const PauseCallback pauseCallback;
	const SetRowCallback setRowCallback;
	const IsPlayingCallback isPlayingCallback;

	struct Variable {
		int components;
		struct {
			const sync_track* track;
			std::string name;
		} component[4];
	};

	struct SyncDeviceDeleter { void operator()(sync_device *d) { sync_destroy_device(d); } };
	std::unique_ptr<sync_device, SyncDeviceDeleter> rocket_;
	std::map<std::string, Variable> vars_;

	float row_ = 0;
};
