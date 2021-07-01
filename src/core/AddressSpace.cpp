#include "AddressSpace.h"
#include <managers/AddressSpaceManager.h>

fs::path CE::AddressSpace::getImagesDirectory() {
	return getAddrSpaceManager()->getProject()->getImagesDirectory() / fs::path(getName());
}
