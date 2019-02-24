#ifndef GRBLDEFINITIONS_H
#define GRBLDEFINITIONS_H

#define MM_PER_INCH (25.40)

#define ARC_ERROR   0.1

#define BOARD_RX_BUFFER_SIZE    127

#define RT_CMD_PROCESS_TIME_MS  200

#define LINE_SEPARATOR_STRING   "\r\n"
#define LINE_SEPARATOR_LENGTH   2

#define RESPONSE_OK             "ok"
#define RESPONSE_ERROR          "error:"
#define RESPONSE_ALARM          "ALARM:"
#define RESPONSE_STATUS_START   "<"
#define RESPONSE_STATUS_END     ">"
#define RESPONSE_STATUS_DELIM   ","
#define RESPONSE_FEEDBACK_START "["
#define RESPONSE_FEEDBACK_END   "]"

#define CMD_PAUSE_STRING        "!"
#define CMD_RESUME_STRING       "~"
#define CMD_STATUS_REQ_STRING   "?"
#define CMD_SOFT_RESET_STRING   "\x18"
#define CMD_SAFETY_DOOR         "@"

#define STATE_IDLE_STRING       "Idle"
#define STATE_RUN_STRING        "Run"
#define STATE_HOLD_STRING       "Hold"
#define STATE_DOOR_STRING       "Door"
#define STATE_HOME_STRING       "Home"
#define STATE_ALARM_STRING      "Alarm"
#define STATE_CHECK_STRING      "Check"

#define STATUS_MACHINE_POS      "MPos:"
#define STATUS_WORK_POS         "WPos:"
#define STATUS_MOTION_NUM       "Buf:"
#define STATUS_CHARACTER_NUM    "RX:"

#define VERSION_STRING          "Grbl "

#define INST_GET_PARAMS         "$$"
#define INST_TOGGLE_CHECK       "$C"
#define INST_KILL_ALARM         "$X"
#define INST_HOMING             "$H"

//Those are command requiring simple question / answer protocol rather than buffered protocol
//For simplification, any $ command is considered EEPROM related
#define INSTRUCTIONS_BLOCKING   "$","G10 L2","G10 L20","G28","G30","G54","G55","G56","G57","G58","G59"

#define END_OF_INSTRUCTION      '\n'

#define GCODE_COMMENTS_DELIM    "(",";","%"


#define GRBL_ERR_REGEXP         "error: Invalid gcode ID:(?<id>\\d\\d)"
#define GRBL_ERR_23             "This G-code command MUST be an integer"
#define GRBL_ERR_24             "Block contains 2 G-code commands that both require the XYZ axis words"
#define GRBL_ERR_25             "Block contains repeated G-code word"
#define GRBL_ERR_26             "Command requires XYZ axis word"
#define GRBL_ERR_27             "Line number cannot exceed 9 999 999"
#define GRBL_ERR_28             "Missing P or L value word"
#define GRBL_ERR_29             "Unsupported work coordinate system"
#define GRBL_ERR_30             "G53 requires either a G0 or G1 mode to be active"
#define GRBL_ERR_31             "Unused axis words and G80 is active"
#define GRBL_ERR_32             "Command requires XYZ axis word"
#define GRBL_ERR_33             "Invalid target (check arc definition)"
#define GRBL_ERR_34             "Error computing arc geometry"
#define GRBL_ERR_35             "Missing the IJK offset word in the selected plane"
#define GRBL_ERR_36             "Unused G-code words detected"
#define GRBL_ERR_37             "G43.1 can only apply on its configured axis"

#define GRBL_PARAM_REPORT_INCHES    13


#endif // GRBLDEFINITIONS_H
