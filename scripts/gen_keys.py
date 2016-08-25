import sys, os

# genereate keys.h file
f = file("src/engine/keys.h", "w")

keynames = []
keynames12 = []
for i in range(0, 512):
	keynames += ["&%d"%i]
	keynames12 += ["&%d"%i]

print >>f, "#ifndef ENGINE_KEYS_H"
print >>f, "#define ENGINE_KEYS_H"

# KEY_EXECUTE already exists on windows platforms
print >>f, "#if defined(CONF_FAMILY_WINDOWS)"
print >>f, "	#undef KEY_EXECUTE"
print >>f, "#endif"


print >>f, '/* AUTO GENERATED! DO NOT EDIT MANUALLY! */'
print >>f, '/* To generate this file, run command: python scripts/gen_keys.py */'
print >>f, ""
print >>f, "#include <SDL_version.h>"
print >>f, ""
print >>f, "enum"
print >>f, "{"

print >>f, "\tKEY_FIRST = 0,"

print >>f, ""
print >>f, "#if SDL_VERSION_ATLEAST(2,0,0)"
print >>f, ""

highestid = 0
for line in open("scripts/SDL_scancode.h"):
	l = line.strip().split("=")
	if len(l) == 2 and "SDL_SCANCODE_" in line:
		key = l[0].strip().replace("SDL_SCANCODE_", "KEY_")
		value = int(l[1].split(",")[0].strip())
		if key[0:2] == "/*":
			continue
		print >>f, "\t%s = %d,"%(key, value)
		
		keynames[value] = key.replace("KEY_", "").lower()
		
		if value > highestid:
			highestid =value

print >>f, "\tKEY_MOUSE_1 = %d,"%(highestid+1); keynames[highestid+1] = "mouse1"
print >>f, "\tKEY_MOUSE_2 = %d,"%(highestid+2); keynames[highestid+2] = "mouse2"
print >>f, "\tKEY_MOUSE_3 = %d,"%(highestid+3); keynames[highestid+3] = "mouse3"
print >>f, "\tKEY_MOUSE_4 = %d,"%(highestid+4); keynames[highestid+4] = "mouse4"
print >>f, "\tKEY_MOUSE_5 = %d,"%(highestid+5); keynames[highestid+5] = "mouse5"
print >>f, "\tKEY_MOUSE_6 = %d,"%(highestid+6); keynames[highestid+6] = "mouse6"
print >>f, "\tKEY_MOUSE_7 = %d,"%(highestid+7); keynames[highestid+7] = "mouse7"
print >>f, "\tKEY_MOUSE_8 = %d,"%(highestid+8); keynames[highestid+8] = "mouse8"
print >>f, "\tKEY_MOUSE_WHEEL_UP = %d,"%(highestid+9); keynames[highestid+9] = "mousewheelup"
print >>f, "\tKEY_MOUSE_WHEEL_DOWN = %d,"%(highestid+10); keynames[highestid+10] = "mousewheeldown"
highestid += 11
print >>f, "\tKEY_GAMEPAD_BUTTON_A = %d,"%(highestid); keynames[highestid] = "gamepad_a" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_B = %d,"%(highestid); keynames[highestid] = "gamepad_b" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_X = %d,"%(highestid); keynames[highestid] = "gamepad_x" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_Y = %d,"%(highestid); keynames[highestid] = "gamepad_y" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_BACK = %d,"%(highestid); keynames[highestid] = "gamepad_back" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_START = %d,"%(highestid); keynames[highestid] = "gamepad_start" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_AXIS_UP = %d,"%(highestid); keynames[highestid] = "gamepad_axis_up" ; highestid += 1
#print >>f, "\tKEY_GAMEPAD_AXIS_DOWN = %d,"%(highestid); keynames[highestid] = "gamepad_axis_down" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_AXIS_LEFT = %d,"%(highestid); keynames[highestid] = "gamepad_axis_left" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_AXIS_RIGHT = %d,"%(highestid); keynames[highestid] = "gamepad_axis_right" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_DPAD_UP = %d,"%(highestid); keynames[highestid] = "gamepad_dpad_up" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_DPAD_DOWN = %d,"%(highestid); keynames[highestid] = "gamepad_dpad_down" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_DPAD_LEFT = %d,"%(highestid); keynames[highestid] = "gamepad_dpad_left" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_DPAD_RIGHT = %d,"%(highestid); keynames[highestid] = "gamepad_dpad_right" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_SHOULDER_LEFT = %d,"%(highestid); keynames[highestid] = "gamepad_shoulder_left" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_SHOULDER_RIGHT = %d,"%(highestid); keynames[highestid] = "gamepad_shoulder_right" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_TRIGGER_LEFT = %d,"%(highestid); keynames[highestid] = "gamepad_trigger_left" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_TRIGGER_RIGHT = %d,"%(highestid); keynames[highestid] = "gamepad_trigger_right" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_LEFTSTICK = %d,"%(highestid); keynames[highestid] = "gamepad_stick_left" ; highestid += 1
#print >>f, "\tKEY_GAMEPAD_BUTTON_RIGHTSTICK = %d,"%(highestid); keynames[highestid] = "gamepad_stick_right" ; highestid += 1
print >>f, ""
print >>f, "#else /* SDL_VERSION_ATLEAST(2,0,0) */"
print >>f, ""

# Yay copypaste

highestid = 0
for line in open("scripts/SDL_keysym.h"):
	l = line.strip().split("=")
	if len(l) == 2 and "SDLK_" in line:
		key = l[0].strip().replace("SDLK_", "KEY_")
		value = int(l[1].split(",")[0].strip())
		if key[0:2] == "/*":
			continue
		print >>f, "\t%s = %d,"%(key, value)
		
		keynames12[value] = key.replace("KEY_", "").lower()
		
		if value > highestid:
			highestid =value

print >>f, "\tKEY_MOUSE_1 = %d,"%(highestid+1); keynames12[highestid+1] = "mouse1"
print >>f, "\tKEY_MOUSE_2 = %d,"%(highestid+2); keynames12[highestid+2] = "mouse2"
print >>f, "\tKEY_MOUSE_3 = %d,"%(highestid+3); keynames12[highestid+3] = "mouse3"
print >>f, "\tKEY_MOUSE_4 = %d,"%(highestid+4); keynames12[highestid+4] = "mouse4"
print >>f, "\tKEY_MOUSE_5 = %d,"%(highestid+5); keynames12[highestid+5] = "mouse5"
print >>f, "\tKEY_MOUSE_6 = %d,"%(highestid+6); keynames12[highestid+6] = "mouse6"
print >>f, "\tKEY_MOUSE_7 = %d,"%(highestid+7); keynames12[highestid+7] = "mouse7"
print >>f, "\tKEY_MOUSE_8 = %d,"%(highestid+8); keynames12[highestid+8] = "mouse8"
print >>f, "\tKEY_MOUSE_WHEEL_UP = %d,"%(highestid+9); keynames12[highestid+9] = "mousewheelup"
print >>f, "\tKEY_MOUSE_WHEEL_DOWN = %d,"%(highestid+10); keynames12[highestid+10] = "mousewheeldown"
highestid += 11
print >>f, "\tKEY_GAMEPAD_BUTTON_A = %d,"%(highestid); keynames12[highestid] = "gamepad_a" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_B = %d,"%(highestid); keynames12[highestid] = "gamepad_b" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_X = %d,"%(highestid); keynames12[highestid] = "gamepad_x" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_Y = %d,"%(highestid); keynames12[highestid] = "gamepad_y" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_BACK = %d,"%(highestid); keynames12[highestid] = "gamepad_back" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_START = %d,"%(highestid); keynames12[highestid] = "gamepad_start" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_AXIS_UP = %d,"%(highestid); keynames12[highestid] = "gamepad_axis_up" ; highestid += 1
#print >>f, "\tKEY_GAMEPAD_AXIS_DOWN = %d,"%(highestid); keynames12[highestid] = "gamepad_axis_down" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_AXIS_LEFT = %d,"%(highestid); keynames12[highestid] = "gamepad_axis_left" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_AXIS_RIGHT = %d,"%(highestid); keynames12[highestid] = "gamepad_axis_right" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_DPAD_UP = %d,"%(highestid); keynames12[highestid] = "gamepad_dpad_up" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_DPAD_DOWN = %d,"%(highestid); keynames12[highestid] = "gamepad_dpad_down" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_DPAD_LEFT = %d,"%(highestid); keynames12[highestid] = "gamepad_dpad_left" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_DPAD_RIGHT = %d,"%(highestid); keynames12[highestid] = "gamepad_dpad_right" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_SHOULDER_LEFT = %d,"%(highestid); keynames12[highestid] = "gamepad_shoulder_left" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_SHOULDER_RIGHT = %d,"%(highestid); keynames12[highestid] = "gamepad_shoulder_right" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_TRIGGER_LEFT = %d,"%(highestid); keynames12[highestid] = "gamepad_trigger_left" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_TRIGGER_RIGHT = %d,"%(highestid); keynames12[highestid] = "gamepad_trigger_right" ; highestid += 1
print >>f, "\tKEY_GAMEPAD_BUTTON_LEFTSTICK = %d,"%(highestid); keynames12[highestid] = "gamepad_stick_left" ; highestid += 1
#print >>f, "\tKEY_GAMEPAD_BUTTON_RIGHTSTICK = %d,"%(highestid); keynames[highestid] = "gamepad_stick_right" ; highestid += 1

print >>f, ""
print >>f, "#endif /* SDL_VERSION_ATLEAST(2,0,0) */"
print >>f, ""

print >>f, "\tKEY_LAST,"

print >>f, "};"
print >>f, ""
print >>f, "#endif"

# generate keynames.c file
f = file("src/engine/client/keynames.h", "w")
print >>f, '/* AUTO GENERATED! DO NOT EDIT MANUALLY! */'
print >>f, '/* To generate this file, run command: python scripts/gen_keys.py */'
print >>f, ""
print >>f, "#include <SDL_version.h>"
print >>f, ''
print >>f, '#ifndef KEYS_INCLUDE'
print >>f, '#error do not include this header!'
print >>f, '#endif'
print >>f, ''
print >>f, "#include <string.h>"
print >>f, ""
print >>f, "const char g_aaKeyStrings[512][20] ="
print >>f, "{"
print >>f, ""
print >>f, "#if SDL_VERSION_ATLEAST(2,0,0)"
print >>f, ""
for n in keynames:
	print >>f, '\t"%s",'%n
print >>f, ""
print >>f, "#else /* SDL_VERSION_ATLEAST(2,0,0) */"
print >>f, ""
for n in keynames12:
	print >>f, '\t"%s",'%n
print >>f, ""
print >>f, "#endif /* SDL_VERSION_ATLEAST(2,0,0) */"
print >>f, ""
print >>f, "};"
print >>f, ""

f.close()

