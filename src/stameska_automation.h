#pragma bss_seg(".automation_uniforms")
static float uniforms[AUTOMATION_SIZE];

#pragma code_seg(".automation_unpack")
void automationUnpack(float t) {
	for (int i = 0; i < AUTOMATION_SIZE; ++i) {
		const int len = automation_lengths_table[i];
		const float * const times = automation_time_data + automation_times_table[i];
		const float * const values = automation_value_data + automation_values_table[i];
		float pv = 0.f;
		float tt = t;
		for (int j = 0; j < len; ++j) {
			const float dt = times[j], dv = values[j];
			if (dt >= tt) {
				tt /= dt;
				pv += dv * tt;
				break;
			}
			tt -= dt;
			pv += dv;
		}
		uniforms[i] = pv;
	}
}
