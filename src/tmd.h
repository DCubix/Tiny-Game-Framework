#ifndef TMD_H
#define TMD_H

#include "Types.h"

namespace tgf {
	struct TMDHeader {
		u8 identifier[3]; // TMD
		u32 vertexCount;
		u32 uvCount;
		u32 faceCount;
		u32 keyframeCount;
		u8 hasUVs;
	};

}

#endif // TMD_H
