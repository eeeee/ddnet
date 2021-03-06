/* (c) whoever wrote the SDL */
#ifndef ENGINE_KEYS_H
#define ENGINE_KEYS_H
#include "SDL.h"
enum
{
	KEY_UNKNOWN = 0,
	KEY_FIRST = 0,
	KEY_BACKSPACE = SDLK_BACKSPACE,
	KEY_TAB = SDLK_TAB,
	KEY_CLEAR = SDLK_CLEAR,
	KEY_RETURN = SDLK_RETURN,
	KEY_PAUSE = SDLK_PAUSE,
	KEY_ESCAPE = SDLK_ESCAPE,
	KEY_SPACE = SDLK_SPACE,
	KEY_EXCLAIM = SDLK_EXCLAIM,
	KEY_QUOTEDBL = SDLK_QUOTEDBL,
	KEY_HASH = SDLK_HASH,
	KEY_DOLLAR = SDLK_DOLLAR,
	KEY_AMPERSAND = SDLK_AMPERSAND,
	KEY_QUOTE = SDLK_QUOTE,
	KEY_LEFTPAREN = SDLK_LEFTPAREN,
	KEY_RIGHTPAREN = SDLK_RIGHTPAREN,
	KEY_ASTERISK = SDLK_ASTERISK,
	KEY_PLUS = SDLK_PLUS,
	KEY_COMMA = SDLK_COMMA,
	KEY_MINUS = SDLK_MINUS,
	KEY_PERIOD = SDLK_PERIOD,
	KEY_SLASH = SDLK_SLASH,
	KEY_0 = SDLK_0,
	KEY_1 = SDLK_1,
	KEY_2 = SDLK_2,
	KEY_3 = SDLK_3,
	KEY_4 = SDLK_4,
	KEY_5 = SDLK_5,
	KEY_6 = SDLK_6,
	KEY_7 = SDLK_7,
	KEY_8 = SDLK_8,
	KEY_9 = SDLK_9,
	KEY_COLON = SDLK_COLON,
	KEY_SEMICOLON = SDLK_SEMICOLON,
	KEY_LESS = SDLK_LESS,
	KEY_EQUALS = SDLK_EQUALS,
	KEY_GREATER = SDLK_GREATER,
	KEY_QUESTION = SDLK_QUESTION,
	KEY_AT = SDLK_AT,
	KEY_LEFTBRACKET = SDLK_LEFTBRACKET,
	KEY_BACKSLASH = SDLK_BACKSLASH,
	KEY_RIGHTBRACKET = SDLK_RIGHTBRACKET,
	KEY_CARET = SDLK_CARET,
	KEY_UNDERSCORE = SDLK_UNDERSCORE,
	KEY_BACKQUOTE = SDLK_BACKQUOTE,
	KEY_a = SDLK_a,
	KEY_b = SDLK_b,
	KEY_c = SDLK_c,
	KEY_d = SDLK_d,
	KEY_e = SDLK_e,
	KEY_f = SDLK_f,
	KEY_g = SDLK_g,
	KEY_h = SDLK_h,
	KEY_i = SDLK_i,
	KEY_j = SDLK_j,
	KEY_k = SDLK_k,
	KEY_l = SDLK_l,
	KEY_m = SDLK_m,
	KEY_n = SDLK_n,
	KEY_o = SDLK_o,
	KEY_p = SDLK_p,
	KEY_q = SDLK_q,
	KEY_r = SDLK_r,
	KEY_s = SDLK_s,
	KEY_t = SDLK_t,
	KEY_u = SDLK_u,
	KEY_v = SDLK_v,
	KEY_w = SDLK_w,
	KEY_x = SDLK_x,
	KEY_y = SDLK_y,
	KEY_z = SDLK_z,
	KEY_DELETE = SDLK_DELETE,
	KEY_WORLD_0 = 160,
	KEY_WORLD_1 = 161,
	KEY_WORLD_2 = 162,
	KEY_WORLD_3 = 163,
	KEY_WORLD_4 = 164,
	KEY_WORLD_5 = 165,
	KEY_WORLD_6 = 166,
	KEY_WORLD_7 = 167,
	KEY_WORLD_8 = 168,
	KEY_WORLD_9 = 169,
	KEY_WORLD_10 = 170,
	KEY_WORLD_11 = 171,
	KEY_WORLD_12 = 172,
	KEY_WORLD_13 = 173,
	KEY_WORLD_14 = 174,
	KEY_WORLD_15 = 175,
	KEY_WORLD_16 = 176,
	KEY_WORLD_17 = 177,
	KEY_WORLD_18 = 178,
	KEY_WORLD_19 = 179,
	KEY_WORLD_20 = 180,
	KEY_WORLD_21 = 181,
	KEY_WORLD_22 = 182,
	KEY_WORLD_23 = 183,
	KEY_WORLD_24 = 184,
	KEY_WORLD_25 = 185,
	KEY_WORLD_26 = 186,
	KEY_WORLD_27 = 187,
	KEY_WORLD_28 = 188,
	KEY_WORLD_29 = 189,
	KEY_WORLD_30 = 190,
	KEY_WORLD_31 = 191,
	KEY_WORLD_32 = 192,
	KEY_WORLD_33 = 193,
	KEY_WORLD_34 = 194,
	KEY_WORLD_35 = 195,
	KEY_WORLD_36 = 196,
	KEY_WORLD_37 = 197,
	KEY_WORLD_38 = 198,
	KEY_WORLD_39 = 199,
	KEY_WORLD_40 = 200,
	KEY_WORLD_41 = 201,
	KEY_WORLD_42 = 202,
	KEY_WORLD_43 = 203,
	KEY_WORLD_44 = 204,
	KEY_WORLD_45 = 205,
	KEY_WORLD_46 = 206,
	KEY_WORLD_47 = 207,
	KEY_WORLD_48 = 208,
	KEY_WORLD_49 = 209,
	KEY_WORLD_50 = 210,
	KEY_WORLD_51 = 211,
	KEY_WORLD_52 = 212,
	KEY_WORLD_53 = 213,
	KEY_WORLD_54 = 214,
	KEY_WORLD_55 = 215,
	KEY_WORLD_56 = 216,
	KEY_WORLD_57 = 217,
	KEY_WORLD_58 = 218,
	KEY_WORLD_59 = 219,
	KEY_WORLD_60 = 220,
	KEY_WORLD_61 = 221,
	KEY_WORLD_62 = 222,
	KEY_WORLD_63 = 223,
	KEY_WORLD_64 = 224,
	KEY_WORLD_65 = 225,
	KEY_WORLD_66 = 226,
	KEY_WORLD_67 = 227,
	KEY_WORLD_68 = 228,
	KEY_WORLD_69 = 229,
	KEY_WORLD_70 = 230,
	KEY_WORLD_71 = 231,
	KEY_WORLD_72 = 232,
	KEY_WORLD_73 = 233,
	KEY_WORLD_74 = 234,
	KEY_WORLD_75 = 235,
	KEY_WORLD_76 = 236,
	KEY_WORLD_77 = 237,
	KEY_WORLD_78 = 238,
	KEY_WORLD_79 = 239,
	KEY_WORLD_80 = 240,
	KEY_WORLD_81 = 241,
	KEY_WORLD_82 = 242,
	KEY_WORLD_83 = 243,
	KEY_WORLD_84 = 244,
	KEY_WORLD_85 = 245,
	KEY_WORLD_86 = 246,
	KEY_WORLD_87 = 247,
	KEY_WORLD_88 = 248,
	KEY_WORLD_89 = 249,
	KEY_WORLD_90 = 250,
	KEY_WORLD_91 = 251,
	KEY_WORLD_92 = 252,
	KEY_WORLD_93 = 253,
	KEY_WORLD_94 = 254,
	KEY_WORLD_95 = 255,
	KEY_KP0 = SDLK_KP0,
	KEY_KP1 = SDLK_KP1,
	KEY_KP2 = SDLK_KP2,
	KEY_KP3 = SDLK_KP3,
	KEY_KP4 = SDLK_KP4,
	KEY_KP5 = SDLK_KP5,
	KEY_KP6 = SDLK_KP6,
	KEY_KP7 = SDLK_KP7,
	KEY_KP8 = SDLK_KP8,
	KEY_KP9 = SDLK_KP9,
	KEY_KP_PERIOD = SDLK_KP_PERIOD,
	KEY_KP_DIVIDE = SDLK_KP_DIVIDE,
	KEY_KP_MULTIPLY = SDLK_KP_MULTIPLY,
	KEY_KP_MINUS = SDLK_KP_MINUS,
	KEY_KP_PLUS = SDLK_KP_PLUS,
	KEY_KP_ENTER = SDLK_KP_ENTER,
	KEY_KP_EQUALS = SDLK_KP_EQUALS,
	KEY_UP = SDLK_UP,
	KEY_DOWN = SDLK_DOWN,
	KEY_RIGHT = SDLK_RIGHT,
	KEY_LEFT = SDLK_LEFT,
	KEY_INSERT = SDLK_INSERT,
	KEY_HOME = SDLK_HOME,
	KEY_END = SDLK_END,
	KEY_PAGEUP = SDLK_PAGEUP,
	KEY_PAGEDOWN = SDLK_PAGEDOWN,
	KEY_F1 = SDLK_F1,
	KEY_F2 = SDLK_F2,
	KEY_F3 = SDLK_F3,
	KEY_F4 = SDLK_F4,
	KEY_F5 = SDLK_F5,
	KEY_F6 = SDLK_F6,
	KEY_F7 = SDLK_F7,
	KEY_F8 = SDLK_F8,
	KEY_F9 = SDLK_F9,
	KEY_F10 = SDLK_F10,
	KEY_F11 = SDLK_F11,
	KEY_F12 = SDLK_F12,
	KEY_F13 = SDLK_F13,
	KEY_F14 = SDLK_F14,
// need this hack because TW assumes F1-F15
// are continious but they are not
	KEY_F15 = SDLK_F12,
//	KEY_F15 = SDLK_F15,
	KEY_NUMLOCK = SDLK_NUMLOCK,
	KEY_CAPSLOCK = SDLK_CAPSLOCK,
	KEY_SCROLLOCK = SDLK_SCROLLOCK,
	KEY_RSHIFT = SDLK_RSHIFT,
	KEY_LSHIFT = SDLK_LSHIFT,
	KEY_RCTRL = SDLK_RCTRL,
	KEY_LCTRL = SDLK_LCTRL,
	KEY_RALT = SDLK_RALT,
	KEY_LALT = SDLK_LALT,
	KEY_RMETA = SDLK_RMETA,
	KEY_LMETA = SDLK_LMETA,
	KEY_LSUPER = SDLK_LSUPER,
	KEY_RSUPER = SDLK_RSUPER,
	KEY_MODE = SDLK_MODE,
	KEY_COMPOSE = SDLK_COMPOSE,
	KEY_HELP = SDLK_HELP,
	KEY_PRINT = SDLK_PRINT,
	KEY_SYSREQ = SDLK_SYSREQ,
	KEY_BREAK = SDLK_BREAK,
	KEY_MENU = SDLK_MENU,
	KEY_POWER = SDLK_POWER,
	KEY_EURO = SDLK_EURO,
	KEY_UNDO = SDLK_UNDO,
	KEY_MOUSE_1 = 323,
	KEY_MOUSE_2 = 324,
	KEY_MOUSE_3 = 325,
	KEY_MOUSE_4 = 326,
	KEY_MOUSE_5 = 327,
	KEY_MOUSE_6 = 328,
	KEY_MOUSE_7 = 329,
	KEY_MOUSE_8 = 330,
	KEY_MOUSE_WHEEL_UP = 331,
	KEY_MOUSE_WHEEL_DOWN = 332,
	KEY_MOUSE_9 = 333,
	KEY_LAST=2047,
};

#endif
