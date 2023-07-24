#ifndef FALLOUT_MAPPER_MP_TARGT_H_
#define FALLOUT_MAPPER_MP_TARGT_H_

namespace fallout {

void target_override_protection();
bool target_overriden();
void target_make_path(char* path, int pid);
int target_init();
int target_exit();
int pick_rot();

} // namespace fallout

#endif /* FALLOUT_MAPPER_MP_TARGT_H_ */
