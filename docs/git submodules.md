# ADD SUBMODULE
# add submodule ImGui from branch docking
git submodule add -b docking https://github.com/ocornut/imgui

# REMOVE SUBMODULE
# Remove the submodule entry from .git/config
git submodule deinit -f external/submodule
# Remove the submodule directory from the superproject's .git/modules directory
rm -rf .git/modules/external/submodule
# Remove the entry in .gitmodules and remove the submodule directory
git rm -f external/submodule