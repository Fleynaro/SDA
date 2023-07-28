import {
  Context,
  AddressSpace,
  Image,
  TestAnalyser,
  VectorImageRW,
  StandartSymbolTable,
} from 'sda-core';

export const createTestObjects = (ctx: Context) => {
  const addressSpace = AddressSpace.New(ctx, 'Test Address Space');
  createTestImage(
    'test 5',
    addressSpace,
    [
      0x48, 0x89, 0x5c, 0x24, 0x08, 0x4c, 0x63, 0x41, 0x14, 0x4c, 0x8b, 0x09, 0x4c, 0x8b, 0xd1,
      0x49, 0x2b, 0xd1, 0x48, 0x8b, 0xc2, 0x48, 0x99, 0x49, 0xf7, 0xf8, 0x48, 0x63, 0xd8, 0x4c,
      0x8b, 0xd8, 0x48, 0x8b, 0xd3, 0x49, 0x0f, 0xaf, 0xd0, 0x42, 0x83, 0x0c, 0x0a, 0xff, 0x83,
      0x79, 0x18, 0xff, 0x75, 0x03, 0x89, 0x41, 0x18, 0x83, 0x79, 0x1c, 0xff, 0x75, 0x06, 0x48,
      0x8d, 0x41, 0x1c, 0xeb, 0x0c, 0x8b, 0x41, 0x14, 0x0f, 0xaf, 0x41, 0x1c, 0x48, 0x98, 0x48,
      0x03, 0x01, 0x44, 0x89, 0x18, 0x48, 0x8b, 0x41, 0x08, 0x44, 0x89, 0x59, 0x1c, 0x80, 0x0c,
      0x18, 0x80, 0x8b, 0x41, 0x20, 0x48, 0x8b, 0x5c, 0x24, 0x08, 0x8d, 0x48, 0xff, 0x33, 0xc8,
      0x81, 0xe1, 0xff, 0xff, 0xff, 0x3f, 0x33, 0xc8, 0x41, 0x89, 0x4a, 0x20, 0xc3,
    ],
  );
  createTestImage(
    'idiv (16 bytes operation)',
    addressSpace,
    [
      0x4c, 0x63, 0x41, 0x14, 0x4c, 0x8b, 0x09, 0x4c, 0x8b, 0xd1, 0x49, 0x2b, 0xd1, 0x48, 0x8b,
      0xc2, 0x48, 0x99, 0x49, 0xf7, 0xf8, 0x48, 0x63, 0xd8, 0x4c, 0x8b, 0xd8, 0x48, 0x8b, 0xd3,
      0x49, 0x0f, 0xaf, 0xd0, 0x42, 0x83, 0x0c, 0x0a, 0xff,
    ],
  );
  createTestImage(
    'xmm registers in incomplete blocks',
    addressSpace,
    [
      0x48, 0xc7, 0xc0, 0x01, 0x00, 0x00, 0x00, 0x48, 0x83, 0xf8, 0x02, 0x0f, 0x10, 0x44, 0x24,
      0x20, 0x75, 0x05, 0x0f, 0x10, 0x44, 0x24, 0x10, 0x0f, 0x11, 0x44, 0x24, 0x10,
    ],
  );
  createTestImage(
    'entity & coords',
    addressSpace,
    [
      0x40, 0x53, 0x48, 0x83, 0xec, 0x50, 0x0f, 0x29, 0x74, 0x24, 0x40, 0xf3, 0x0f, 0x10, 0x35,
      0x21, 0x46, 0xf7, 0x01, 0x48, 0x8b, 0xd9, 0x0f, 0x29, 0x7c, 0x24, 0x30, 0xf3, 0x0f, 0x10,
      0x3d, 0x15, 0x46, 0xf7, 0x01, 0x8b, 0xca, 0x44, 0x0f, 0x29, 0x44, 0x24, 0x20, 0xf3, 0x44,
      0x0f, 0x10, 0x05, 0x08, 0x46, 0xf7, 0x01, 0xe8, 0x5f, 0xe0, 0xfd, 0xff, 0x48, 0x85, 0xc0,
      0x74, 0x14, 0x0f, 0x28, 0x70, 0x70, 0x0f, 0x28, 0xfe, 0x44, 0x0f, 0x28, 0xc6, 0x0f, 0xc6,
      0xfe, 0x55, 0x44, 0x0f, 0xc6, 0xc6, 0xaa, 0xf3, 0x0f, 0x11, 0x33, 0x0f, 0x28, 0x74, 0x24,
      0x40, 0xf3, 0x0f, 0x11, 0x7b, 0x08, 0x0f, 0x28, 0x7c, 0x24, 0x30, 0x48, 0x8b, 0xc3, 0xf3,
      0x44, 0x0f, 0x11, 0x43, 0x10, 0x44, 0x0f, 0x28, 0x44, 0x24, 0x20, 0x48, 0x83, 0xc4, 0x50,
      0x5b, 0xc3, 0x90, 0x48,
    ],
  );
  createTestImage(
    'GET_ANGLE_BETWEEN_2D_VECTORS',
    addressSpace,
    [
      0x48, 0x83, 0xec, 0x38, 0x0f, 0x29, 0x74, 0x24, 0x20, 0x0f, 0x28, 0xf0, 0x0f, 0x28, 0xe1,
      0xf3, 0x0f, 0x59, 0xc9, 0xf3, 0x0f, 0x59, 0xf6, 0xf3, 0x0f, 0x59, 0xe3, 0x0f, 0x28, 0xea,
      0xf3, 0x0f, 0x58, 0xf1, 0xf3, 0x0f, 0x59, 0xc5, 0x0f, 0x57, 0xd2, 0x0f, 0x2f, 0xf2, 0xf3,
      0x0f, 0x58, 0xc4, 0x76, 0x09, 0x0f, 0x57, 0xe4, 0xf3, 0x0f, 0x51, 0xe6, 0xeb, 0x03, 0x0f,
      0x28, 0xe2, 0xf3, 0x0f, 0x59, 0xed, 0xf3, 0x0f, 0x59, 0xdb, 0xf3, 0x0f, 0x58, 0xeb, 0x0f,
      0x2f, 0xea, 0x76, 0x09, 0x0f, 0x57, 0xc9, 0xf3, 0x0f, 0x51, 0xcd, 0xeb, 0x03, 0x0f, 0x28,
      0xca, 0xf3, 0x0f, 0x10, 0x1d, 0x59, 0xbf, 0xdf, 0x00, 0xf3, 0x0f, 0x59, 0xcc, 0xf3, 0x0f,
      0x5e, 0xc1, 0x0f, 0x2f, 0xc3, 0x73, 0x03, 0x0f, 0x28, 0xc3, 0xf3, 0x0f, 0x10, 0x0d, 0xd5,
      0xb5, 0xeb, 0x00, 0x0f, 0x2f, 0xc1, 0x76, 0x03, 0x0f, 0x28, 0xc1, 0x0f, 0x2f, 0xc3, 0x76,
      0x0f, 0x0f, 0x2f, 0xc1, 0x73, 0x12, 0xe8, 0x12, 0x4f, 0xcc, 0x00, 0x0f, 0x28, 0xd0, 0xeb,
      0x08, 0xf3, 0x0f, 0x10, 0x15, 0xed, 0x18, 0xe0, 0x00, 0xf3, 0x0f, 0x59, 0x15, 0xf1, 0xbe,
      0xdf, 0x00, 0x0f, 0x28, 0x74, 0x24, 0x20, 0x0f, 0x28, 0xc2, 0x48, 0x83, 0xc4, 0x38, 0xc3,
    ],
  );
  createTestImage(
    'Evklid',
    addressSpace,
    [
      0x89, 0xc8, 0x89, 0xd3, 0x83, 0xf8, 0x00, 0x7d, 0x02, 0xf7, 0xd8, 0x83, 0xfb, 0x00, 0x7d,
      0x02, 0xf7, 0xdb, 0x39, 0xd8, 0x7d, 0x01, 0x93, 0x83, 0xfb, 0x00, 0x74, 0x04, 0x29, 0xd8,
      0xeb, 0xf2, 0x89, 0x04, 0x24, 0x89, 0x1c, 0x24,
    ],
  );
  createTestImage(
    'JMP function',
    addressSpace,
    [
      0x40, 0x53, 0x48, 0x83, 0xec, 0x20, 0x8b, 0xda, 0x83, 0xfa, 0x0a, 0x7e, 0x10, 0x8d, 0x42,
      0xf5, 0x83, 0xf8, 0x0d, 0x77, 0x05, 0x83, 0xc3, 0x19, 0xeb, 0x03, 0x83, 0xeb, 0x0e, 0xe8,
      0x46, 0xca, 0xfe, 0xff, 0x48, 0x85, 0xc0, 0x74, 0x2c, 0x83, 0xfb, 0x31, 0x77, 0x27, 0x48,
      0xba, 0xff, 0xff, 0xff, 0xff, 0xff, 0x43, 0x03, 0x00, 0x48, 0x0f, 0xa3, 0xda, 0x73, 0x17,
      0x48, 0x8b, 0x48, 0x48, 0x4c, 0x8b, 0xc0, 0x8b, 0xd3, 0x48, 0x83, 0xc1, 0x40, 0x48, 0x83,
      0xc4, 0x20, 0x5b, 0xe9, 0x0d, 0x10, 0x91, 0xff, 0x33, 0xc0, 0x48, 0x83, 0xc4, 0x20, 0x5b,
      0xc3, 0xcc,
    ],
  );
  createTestImage(
    'RAX request but no EAX',
    addressSpace,
    [
      0x48, 0x83, 0xec, 0x28, 0xe8, 0x1b, 0xb2, 0xfe, 0xff, 0x48, 0x85, 0xc0, 0x74, 0x0e, 0x48,
      0x8b, 0x40, 0x20, 0x0f, 0xb6, 0x80, 0x18, 0x05, 0x00, 0x00, 0x83, 0xe0, 0x1f, 0x48, 0x83,
      0xc4, 0x28, 0xc3, 0x90, 0x89, 0xed,
    ],
  );
  createTestImage(
    'test 105',
    addressSpace,
    [
      0x48, 0x83, 0xec, 0x28, 0x8b, 0x44, 0x24, 0x38, 0x48, 0x8d, 0x54, 0x24, 0x40, 0xc7, 0x44,
      0x24, 0x40, 0xff, 0xff, 0x00, 0x00, 0x0d, 0xff, 0xff, 0xff, 0x0f, 0x25, 0xff, 0xff, 0xff,
      0x0f, 0x89, 0x44, 0x24, 0x38, 0xe8, 0x50, 0x8f, 0x8b, 0x00, 0x0f, 0xb7, 0x4c, 0x24, 0x40,
      0x66, 0x89, 0x4c, 0x24, 0x38, 0x8b, 0x4c, 0x24, 0x38, 0x4c, 0x8b, 0xc0, 0x81, 0xc9, 0x00,
      0x00, 0xff, 0x0f, 0x33, 0xc0, 0x0f, 0xba, 0xf1, 0x1c, 0x66, 0x81, 0xf9, 0xff, 0xff, 0x74,
      0x10, 0x4d, 0x85, 0xc0, 0x74, 0x0b, 0x41, 0x0f, 0xb6, 0x80, 0x18, 0x05, 0x00, 0x00, 0x83,
      0xe0, 0x1f, 0x48, 0x83, 0xc4, 0x28, 0xc3, 0xcc, 0x54, 0x48,
    ],
  );
  createTestImage(
    'matrix vector coords multiplied',
    addressSpace,
    [
      0x48, 0x8b, 0xc4, 0x48, 0x89, 0x58, 0x08, 0x48, 0x89, 0x70, 0x10, 0x57, 0x48, 0x83, 0xec,
      0x60, 0x0f, 0x29, 0x70, 0xe8, 0xf3, 0x0f, 0x10, 0x35, 0xb4, 0x35, 0xf7, 0x01, 0x0f, 0x29,
      0x78, 0xd8, 0xf3, 0x0f, 0x10, 0x3d, 0xac, 0x35, 0xf7, 0x01, 0x48, 0x8b, 0xd9, 0x8b, 0xca,
      0x41, 0x8a, 0xf0, 0x44, 0x0f, 0x29, 0x40, 0xc8, 0x44, 0x0f, 0x29, 0x48, 0xb8, 0xf3, 0x44,
      0x0f, 0x10, 0x0d, 0x89, 0x35, 0xf7, 0x01, 0xe8, 0x1c, 0xd0, 0xfd, 0xff, 0x48, 0x8b, 0xf8,
      0x48, 0x85, 0xc0, 0x0f, 0x84, 0x96, 0x00, 0x00, 0x00, 0x48, 0x8b, 0x10, 0x48, 0x8b, 0xc8,
      0xff, 0x92, 0x68, 0x03, 0x00, 0x00, 0xf3, 0x44, 0x0f, 0x10, 0x08, 0xf3, 0x0f, 0x10, 0x70,
      0x04, 0xf3, 0x0f, 0x10, 0x78, 0x08, 0x40, 0x84, 0xf6, 0x74, 0x76, 0x48, 0x8b, 0x07, 0x48,
      0x8b, 0xcf, 0xff, 0x90, 0x68, 0x03, 0x00, 0x00, 0x44, 0x0f, 0x28, 0x47, 0x60, 0x0f, 0x28,
      0x7f, 0x70, 0x0f, 0x28, 0xaf, 0x80, 0x00, 0x00, 0x00, 0x0f, 0x57, 0xf6, 0x66, 0x0f, 0x70,
      0x08, 0x00, 0x66, 0x0f, 0x70, 0x00, 0x55, 0x66, 0x0f, 0x70, 0x18, 0xaa, 0x41, 0x0f, 0x28,
      0xe0, 0x0f, 0x28, 0xd7, 0x0f, 0x15, 0xfe, 0x44, 0x0f, 0x15, 0xc5, 0x0f, 0x14, 0xd6, 0x0f,
      0x14, 0xe5, 0x44, 0x0f, 0x14, 0xc7, 0x44, 0x0f, 0x28, 0xcc, 0x44, 0x0f, 0x15, 0xca, 0x0f,
      0x14, 0xe2, 0x44, 0x0f, 0x59, 0xc3, 0x44, 0x0f, 0x59, 0xc8, 0x0f, 0x59, 0xe1, 0x44, 0x0f,
      0x58, 0xcc, 0x45, 0x0f, 0x58, 0xc8, 0x41, 0x0f, 0x28, 0xf1, 0x41, 0x0f, 0x28, 0xf9, 0x41,
      0x0f, 0xc6, 0xf1, 0x55, 0x41, 0x0f, 0xc6, 0xf9, 0xaa, 0x48, 0x8b, 0x74, 0x24, 0x78, 0x44,
      0x0f, 0x28, 0x44, 0x24, 0x30, 0xf3, 0x44, 0x0f, 0x11, 0x0b, 0x44, 0x0f, 0x28, 0x4c, 0x24,
      0x20, 0xf3, 0x0f, 0x11, 0x73, 0x08, 0x0f, 0x28, 0x74, 0x24, 0x50, 0xf3, 0x0f, 0x11, 0x7b,
      0x10, 0x48, 0x8b, 0xc3, 0x48, 0x8b, 0x5c, 0x24, 0x70, 0x0f, 0x28, 0x7c, 0x24, 0x40, 0x48,
      0x83, 0xc4, 0x60, 0x5f, 0xc3,
    ],
  );
  createTestImage(
    'hard stack memory copying',
    addressSpace,
    [
      0x40, 0x55, 0x48, 0x8d, 0x6c, 0x24, 0xa9, 0x48, 0x81, 0xec, 0xd0, 0x00, 0x00, 0x00, 0xf2,
      0x0f, 0x10, 0x4d, 0xc7, 0x45, 0x33, 0xc0, 0x48, 0x8d, 0x45, 0x17, 0x44, 0x89, 0x45, 0xbf,
      0xf2, 0x0f, 0x11, 0x4d, 0x27, 0xf2, 0x0f, 0x10, 0x4d, 0xc7, 0x48, 0x89, 0x44, 0x24, 0x28,
      0x48, 0x8d, 0x45, 0xd7, 0x4c, 0x8d, 0x4d, 0xf7, 0x0f, 0x10, 0x45, 0xb7, 0xf2, 0x0f, 0x11,
      0x4d, 0xe7, 0xf2, 0x0f, 0x10, 0x4d, 0xc7, 0x44, 0x89, 0x45, 0xbf, 0x48, 0x8d, 0x15, 0x5e,
      0xc5, 0x6c, 0x01, 0x48, 0x89, 0x44, 0x24, 0x20, 0x0f, 0x29, 0x45, 0x17, 0x0f, 0x10, 0x45,
      0xb7, 0xf2, 0x0f, 0x11, 0x4d, 0x07, 0xf2, 0x0f, 0x10, 0x4d, 0xc7, 0x44, 0x89, 0x45, 0xbf,
      0x4c, 0x8d, 0x45, 0x37, 0x0f, 0x29, 0x45, 0xd7, 0x0f, 0x10, 0x45, 0xb7, 0xc7, 0x45, 0xbf,
      0x01, 0x00, 0x00, 0x00, 0xf2, 0x0f, 0x11, 0x4d, 0x47, 0x0f, 0x29, 0x45, 0xf7, 0x66, 0x0f,
      0x6e, 0xc1, 0x48, 0x8d, 0x0d, 0xc8, 0xf4, 0xe2, 0x01, 0xf3, 0x0f, 0xe6, 0xc0, 0xf2, 0x0f,
      0x11, 0x45, 0xb7, 0x0f, 0x10, 0x45, 0xb7, 0x0f, 0x29, 0x45, 0x37, 0xe8, 0xaa, 0xde, 0xfe,
      0xff, 0x48, 0x81, 0xc4, 0xd0, 0x00, 0x00, 0x00, 0x5d, 0xc3, 0xcc,
    ],
  );
  createTestImage(
    'Matrix_FillWithVectorsAndMul',
    addressSpace,
    [
      0x4c, 0x8b, 0xdc, 0x48, 0x81, 0xec, 0xb8, 0x00, 0x00, 0x00, 0x0f, 0x28, 0x02, 0x48, 0x8b,
      0x94, 0x24, 0xf0, 0x00, 0x00, 0x00, 0x48, 0x8b, 0x84, 0x24, 0xe0, 0x00, 0x00, 0x00, 0x41,
      0x0f, 0x29, 0x73, 0xe8, 0x41, 0x0f, 0x29, 0x7b, 0xd8, 0x45, 0x0f, 0x29, 0x43, 0xc8, 0x4d,
      0x8d, 0x1b, 0x66, 0x0f, 0x70, 0x22, 0x55, 0x66, 0x0f, 0x70, 0x32, 0xaa, 0x66, 0x0f, 0x70,
      0x2a, 0x00, 0x45, 0x0f, 0x29, 0x4b, 0xb8, 0x45, 0x0f, 0x29, 0x53, 0xa8, 0x45, 0x0f, 0x29,
      0x5b, 0x98, 0x45, 0x0f, 0x29, 0x63, 0x88, 0x44, 0x0f, 0x29, 0x6c, 0x24, 0x30, 0x44, 0x0f,
      0x28, 0x28, 0x48, 0x8b, 0x84, 0x24, 0xe8, 0x00, 0x00, 0x00, 0x66, 0x0f, 0x70, 0x08, 0x00,
      0x66, 0x0f, 0x70, 0x18, 0xaa, 0x44, 0x0f, 0x29, 0x74, 0x24, 0x20, 0x45, 0x0f, 0x28, 0x30,
      0x44, 0x0f, 0x29, 0x7c, 0x24, 0x10, 0x4c, 0x8b, 0x84, 0x24, 0xf8, 0x00, 0x00, 0x00, 0x66,
      0x45, 0x0f, 0x70, 0x00, 0x00, 0x66, 0x41, 0x0f, 0x70, 0x38, 0x55, 0x66, 0x45, 0x0f, 0x70,
      0x08, 0xaa, 0x45, 0x0f, 0x28, 0x39, 0x0f, 0x29, 0x04, 0x24, 0x41, 0x0f, 0x28, 0xd6, 0x4c,
      0x8b, 0x8c, 0x24, 0x00, 0x01, 0x00, 0x00, 0x66, 0x0f, 0x70, 0x00, 0x55, 0x66, 0x45, 0x0f,
      0x70, 0x11, 0x00, 0x66, 0x45, 0x0f, 0x70, 0x19, 0x55, 0x0f, 0x59, 0xd0, 0x0f, 0x28, 0x04,
      0x24, 0x0f, 0x59, 0xc1, 0x41, 0x0f, 0x28, 0xcf, 0x0f, 0x59, 0xcb, 0x66, 0x45, 0x0f, 0x70,
      0x21, 0xaa, 0x0f, 0x58, 0xd0, 0x41, 0x0f, 0x28, 0xde, 0x0f, 0x59, 0xdc, 0x0f, 0x28, 0x24,
      0x24, 0x0f, 0x28, 0xc4, 0x0f, 0x58, 0xd1, 0x0f, 0x59, 0xc5, 0x41, 0x0f, 0x28, 0xcf, 0x0f,
      0x59, 0xce, 0x0f, 0x58, 0xd8, 0x41, 0x0f, 0x28, 0x73, 0xe8, 0x0f, 0x28, 0xc4, 0x0f, 0x29,
      0x11, 0x41, 0x0f, 0x28, 0xd6, 0x41, 0x0f, 0x59, 0xe2, 0x45, 0x0f, 0x59, 0xf3, 0x0f, 0x58,
      0xd9, 0x41, 0x0f, 0x58, 0xe5, 0x45, 0x0f, 0x28, 0x53, 0xa8, 0x45, 0x0f, 0x28, 0x5b, 0x98,
      0x44, 0x0f, 0x28, 0x6c, 0x24, 0x30, 0x0f, 0x59, 0xd7, 0x41, 0x0f, 0x59, 0xc0, 0x41, 0x0f,
      0x58, 0xe6, 0x0f, 0x58, 0xd0, 0x41, 0x0f, 0x28, 0x7b, 0xd8, 0x45, 0x0f, 0x28, 0x43, 0xc8,
      0x44, 0x0f, 0x28, 0x74, 0x24, 0x20, 0x41, 0x0f, 0x28, 0xcf, 0x0f, 0x29, 0x59, 0x10, 0x45,
      0x0f, 0x59, 0xfc, 0x41, 0x0f, 0x59, 0xc9, 0x45, 0x0f, 0x28, 0x4b, 0xb8, 0x45, 0x0f, 0x28,
      0x63, 0x88, 0x41, 0x0f, 0x58, 0xe7, 0x0f, 0x58, 0xd1, 0x44, 0x0f, 0x28, 0x7c, 0x24, 0x10,
      0x0f, 0x29, 0x61, 0x30, 0x0f, 0x29, 0x51, 0x20, 0x49, 0x8b, 0xe3, 0xc3,
    ],
  );
  createTestImage(
    'SET_ENTITY_ANIM_SPEED',
    addressSpace,
    [
      0x48, 0x8b, 0xc4, 0x48, 0x89, 0x58, 0x08, 0x48, 0x89, 0x70, 0x10, 0x48, 0x89, 0x78, 0x18,
      0x4c, 0x89, 0x70, 0x20, 0x55, 0x48, 0x8b, 0xec, 0x48, 0x83, 0xec, 0x50, 0x0f, 0x29, 0x70,
      0xe8, 0x4c, 0x8d, 0x0d, 0x9a, 0xba, 0xe9, 0x00, 0x49, 0x8b, 0xf0, 0x0f, 0x28, 0xf3, 0x4c,
      0x8b, 0xf2, 0x8b, 0xf9, 0xe8, 0x3e, 0xcd, 0x00, 0x00, 0x4c, 0x8b, 0xc8, 0x48, 0x85, 0xc0,
      0x74, 0x5a, 0x48, 0x8b, 0x40, 0x10, 0x48, 0x85, 0xc0, 0x74, 0x0e, 0x8a, 0x88, 0x14, 0x02,
      0x00, 0x00, 0xc0, 0xe9, 0x06, 0x80, 0xe1, 0x01, 0xeb, 0x02, 0x32, 0xc9, 0x84, 0xc9, 0x75,
      0x3d, 0x8b, 0x05, 0xcb, 0x0b, 0x3c, 0x01, 0xa8, 0x01, 0x75, 0x16, 0x83, 0xc8, 0x01, 0x89,
      0x05, 0xbe, 0x0b, 0x3c, 0x01, 0xb8, 0x88, 0xc0, 0x68, 0x7e, 0x89, 0x05, 0xaf, 0x0b, 0x3c,
      0x01, 0xeb, 0x06, 0x8b, 0x05, 0xa7, 0x0b, 0x3c, 0x01, 0x48, 0x8d, 0x55, 0xe0, 0x0f, 0x28,
      0xd6, 0x49, 0x8b, 0xc9, 0x89, 0x45, 0xe0, 0xe8, 0xe9, 0x9a, 0xe1, 0xff, 0xe9, 0xc8, 0x00,
      0x00, 0x00, 0x41, 0xb0, 0x01, 0x41, 0xb9, 0x07, 0x00, 0x00, 0x00, 0x8b, 0xcf, 0x41, 0x8a,
      0xd0, 0xe8, 0x5d, 0xc1, 0x00, 0x00, 0x48, 0x8b, 0xd8, 0x48, 0x85, 0xc0, 0x74, 0x45, 0x48,
      0x8b, 0xd6, 0x33, 0xc9, 0xe8, 0x47, 0x17, 0x7e, 0x00, 0x49, 0x8b, 0xd6, 0x33, 0xc9, 0x89,
      0x45, 0xe0, 0xe8, 0x3a, 0x17, 0x7e, 0x00, 0x4c, 0x8d, 0x4d, 0xec, 0x89, 0x45, 0xe4, 0x48,
      0x8d, 0x45, 0xe8, 0x4c, 0x8d, 0x45, 0xe0, 0x48, 0x8d, 0x55, 0xe4, 0x48, 0x8b, 0xcb, 0x48,
      0x89, 0x44, 0x24, 0x20, 0xe8, 0x62, 0xc0, 0x00, 0x00, 0x84, 0xc0, 0x74, 0x0a, 0x44, 0x8a,
      0x4d, 0xe8, 0x44, 0x8b, 0x45, 0xec, 0xeb, 0x5d, 0x41, 0xb9, 0x07, 0x00, 0x00, 0x00, 0x45,
      0x33, 0xc0, 0xb2, 0x01, 0x8b, 0xcf, 0xe8, 0xfe, 0xc0, 0x00, 0x00, 0x48, 0x8b, 0xd8, 0x48,
      0x85, 0xc0, 0x74, 0x4e, 0x48, 0x8b, 0xd6, 0x33, 0xc9, 0xe8, 0xe8, 0x16, 0x7e, 0x00, 0x49,
      0x8b, 0xd6, 0x33, 0xc9, 0x89, 0x45, 0xec, 0xe8, 0xdb, 0x16, 0x7e, 0x00, 0x4c, 0x8d, 0x4d,
      0xe0, 0x89, 0x45, 0xe8, 0x48, 0x8d, 0x45, 0xe4, 0x4c, 0x8d, 0x45, 0xec, 0x48, 0x8d, 0x55,
      0xe8, 0x48, 0x8b, 0xcb, 0x48, 0x89, 0x44, 0x24, 0x20, 0xe8, 0x03, 0xc0, 0x00, 0x00, 0x84,
      0xc0, 0x74, 0x13, 0x44, 0x8a, 0x4d, 0xe4, 0x44, 0x8b, 0x45, 0xe0, 0x0f, 0x28, 0xce, 0x48,
      0x8b, 0xcb, 0xe8, 0x74, 0x69, 0x8e, 0xff, 0x48, 0x8b, 0x5c, 0x24, 0x60, 0x48, 0x8b, 0x74,
      0x24, 0x68, 0x48, 0x8b, 0x7c, 0x24, 0x70, 0x0f, 0x28, 0x74, 0x24, 0x40, 0x4c, 0x8b, 0x74,
      0x24, 0x78, 0x48, 0x83, 0xc4, 0x50, 0x5d, 0xc3,
    ],
  );
};

const createTestImage = (name: string, addressSpace: AddressSpace, data: number[]) => {
  const analyser = TestAnalyser.New();
  const rw = VectorImageRW.New(data);
  const symbolTable = StandartSymbolTable.New(addressSpace.context, '');
  const testImage = Image.New(addressSpace.context, rw, analyser, name, symbolTable);
  testImage.analyse();
  addressSpace.images = [...addressSpace.images, testImage];
  return testImage;
};
