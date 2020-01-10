#pragma once
//Offsets
#define OFFSET_ENTITYLIST			0x183F118
#define OFFSET_LOCAL				0x1038934
#define OFFSET_LOCAL_ENT			0x1D9AB98 //(grapple_swingPullAngle+0xA8)0x022E93F8


#define OFFSET_TEAM					0x3F0
#define OFFSET_HEALTH				0x3E0
#define OFFSET_NAME					0x521
#define OFFSET_SHIELD				0x170


#define OFFSET_ORIGIN				0x14C
#define OFFSET_BONES				0xEE0
#define OFFSET_AIMPUNCH				0x20C0
#define OFFSET_CAMERAPOS			0x1B64
#define OFFSET_VIEWANGLES			0x2188
#define OFFSET_OBSERVING_TARGET		0x2F40

#define OFFSET_MATRIX 0x1A93D0
#define OFFSET_RENDER 0xCAB13F8


//W2S pattern (view render and view matrix) 48 89 ?? ?? ?? 57 48 83 EC ?? 48 8B D9 48 8B FA 48 8B 0D ?? ?? ?? ?? 33 D2 48 8B 01 FF



//#define OFFSET_BULLET_VELOCITY		0x1C90
//#define OFFSET_ENTITY_VELOCITY		0x41C
//#define OFFSET_LAST_WEAPON			0x1704
//m_vecVelocity = 0x41C
//ViewRender = 0xD4138F0
//ViewAngle = 0x2188
//m_bleedoutState = 0x2348
//EntityCount = 0xC01BAA0
//#define OFFSET_GLOW_FADE			0x2D0
//#define OFFSET_GLOW_ENABLE			0x390
//#define OFFSET_GLOW_CONTEXT			0x310
//#define OFFSET_GLOW_FADE			0x2B8
//#define OFFSET_GLOW_FADE_MAX		(OFFSET_GLOW_FADE+0x18)
//#define OFFSET_GLOW_COLORS			0x1D0
//#define OFFSET_GLOW_MAGIC			0x278
//#define OFFSET_GLOW_RANGE			0x2FC
//#define OFFSET_GLOW_BORDER_SIZE		0x280