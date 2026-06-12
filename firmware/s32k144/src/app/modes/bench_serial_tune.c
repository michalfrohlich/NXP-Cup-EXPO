#include "../app_internal.h"

static void serial_tune_draw(const SerialTuneState_t *st)
{
    char lineBuf[17];
    int scaledValue;
    int wholePart;
    int fracPart;

    if (st == NULL_PTR)
    {
        return;
    }

    DisplayClear();
    switch (st->screen)
    {
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP:
            DisplayTextPadded(0U, "EDIT SERVO CLMP");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF:
            DisplayTextPadded(0U, "EDIT SERVO LPF");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST:
            DisplayTextPadded(0U, "EDIT VISION MIN");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH:
            DisplayTextPadded(0U, "EDIT EDGE HIGH");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW:
            DisplayTextPadded(0U, "EDIT EDGE LOW");
            break;
        case SERIAL_TUNE_SCREEN_VISION_MENU:
            DisplayTextPadded(0U, "VISION TUNE");
            break;
        case SERIAL_TUNE_SCREEN_SERVO_MENU:
            DisplayTextPadded(0U, "SERVO TUNE MENU");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_KP:
            DisplayTextPadded(0U, "EDIT KP");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_KI:
            DisplayTextPadded(0U, "EDIT KI");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_KD:
            DisplayTextPadded(0U, "EDIT KD");
            break;
        case SERIAL_TUNE_SCREEN_MENU:
            DisplayTextPadded(0U, "CONNECTED MENU");
            break;
        case SERIAL_TUNE_SCREEN_WAIT:
        default:
            DisplayTextPadded(0U, "WAITING SERIAL");
            break;
    }

    if ((st->screen == SERIAL_TUNE_SCREEN_VISION_MENU) ||
        (st->screen == SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST) ||
        (st->screen == SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH) ||
        (st->screen == SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW))
    {
        (void)snprintf(lineBuf, sizeof(lineBuf), "MIN:%5u", (unsigned int)st->visionMinContrast);
        DisplayTextPadded(1U, lineBuf);
        (void)snprintf(lineBuf, sizeof(lineBuf), "HIGH:%3u%%", (unsigned int)st->visionEdgeHighPct);
        DisplayTextPadded(2U, lineBuf);
        (void)snprintf(lineBuf, sizeof(lineBuf), "LOW:%4u%%", (unsigned int)st->visionEdgeLowPct);
        DisplayTextPadded(3U, lineBuf);
        DisplayRefresh();
        return;
    }

    if ((st->screen == SERIAL_TUNE_SCREEN_SERVO_MENU) ||
        (st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP) ||
        (st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF))
    {
        (void)snprintf(lineBuf, sizeof(lineBuf), "CLAMP:%4d", (int)st->servoClamp);
        DisplayTextPadded(1U, lineBuf);

        scaledValue = (int)((st->servoLpfAlpha * 100.0f) + 0.5f);
        if (scaledValue < 0)
        {
            scaledValue = 0;
        }
        if (scaledValue > 99999)
        {
            scaledValue = 99999;
        }
        wholePart = scaledValue / 100;
        fracPart = scaledValue % 100;
        (void)snprintf(lineBuf, sizeof(lineBuf), "LPF:%5d.%02d", wholePart, fracPart);
        DisplayTextPadded(2U, lineBuf);
        DisplayTextPadded(3U, "");
        DisplayRefresh();
        return;
    }

    scaledValue = (int)((st->kp * 100.0f) + 0.5f);
    if (scaledValue < 0)
    {
        scaledValue = 0;
    }
    if (scaledValue > 99999)
    {
        scaledValue = 99999;
    }
    wholePart = scaledValue / 100;
    fracPart = scaledValue % 100;
    (void)snprintf(lineBuf, sizeof(lineBuf), "KP:%3d.%02d", wholePart, fracPart);
    DisplayTextPadded(1U, lineBuf);

    scaledValue = (int)((st->ki * 100.0f) + 0.5f);
    if (scaledValue < 0)
    {
        scaledValue = 0;
    }
    if (scaledValue > 99999)
    {
        scaledValue = 99999;
    }
    wholePart = scaledValue / 100;
    fracPart = scaledValue % 100;
    (void)snprintf(lineBuf, sizeof(lineBuf), "KI:%3d.%02d", wholePart, fracPart);
    DisplayTextPadded(2U, lineBuf);

    scaledValue = (int)((st->kd * 100.0f) + 0.5f);
    if (scaledValue < 0)
    {
        scaledValue = 0;
    }
    if (scaledValue > 99999)
    {
        scaledValue = 99999;
    }
    wholePart = scaledValue / 100;
    fracPart = scaledValue % 100;
    (void)snprintf(lineBuf, sizeof(lineBuf), "KD:%3d.%02d", wholePart, fracPart);
    DisplayTextPadded(3U, lineBuf);

    DisplayRefresh();
}

static void serial_tune_print_pid_menu(void)
{
    UartHost_WriteLine("");
    UartHost_WriteLine("=== PID Tune Menu ===");
    UartHost_WriteLine("p - edit KP");
    UartHost_WriteLine("i - edit KI");
    UartHost_WriteLine("d - edit KD");
    UartHost_WriteLine("s - servo menu");
    UartHost_WriteLine("v - vision menu");
    UartHost_WriteString("menu> ");
}

static void serial_tune_print_servo_menu(void)
{
    UartHost_WriteLine("");
    UartHost_WriteLine("=== Servo Tune Menu ===");
    UartHost_WriteLine("c - edit clamp");
    UartHost_WriteLine("l - edit LPF");
    UartHost_WriteLine("p - PID menu");
    UartHost_WriteString("servo> ");
}

static void serial_tune_print_vision_menu(void)
{
    char lineBuf[32];

    UartHost_WriteLine("");
    UartHost_WriteLine("=== Vision Tune Menu ===");
    UartHost_WriteLine("Current:");
    (void)snprintf(lineBuf, sizeof(lineBuf), "minContrast: %u", (unsigned int)g_serialTune.visionMinContrast);
    UartHost_WriteLine(lineBuf);
    (void)snprintf(lineBuf, sizeof(lineBuf), "edgeHighPct: %u", (unsigned int)g_serialTune.visionEdgeHighPct);
    UartHost_WriteLine(lineBuf);
    (void)snprintf(lineBuf, sizeof(lineBuf), "edgeLowPct: %u", (unsigned int)g_serialTune.visionEdgeLowPct);
    UartHost_WriteLine(lineBuf);
    UartHost_WriteLine("");
    UartHost_WriteLine("m - edit min contrast");
    UartHost_WriteLine("h - edit high edge %");
    UartHost_WriteLine("l - edit low edge %");
    UartHost_WriteLine("p - PID menu");
    UartHost_WriteLine("q - show menu");
    UartHost_WriteString("vision> ");
}

static void serial_tune_print_edit_help(SerialTuneScreen_t screen)
{
    switch (screen)
    {
        case SERIAL_TUNE_SCREEN_EDIT_KP:
            UartHost_WriteLine("");
            UartHost_WriteLine("Editing KP");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_KI:
            UartHost_WriteLine("");
            UartHost_WriteLine("Editing KI");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_KD:
            UartHost_WriteLine("");
            UartHost_WriteLine("Editing KD");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP:
            UartHost_WriteLine("");
            UartHost_WriteLine("Editing SERVO CLAMP");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF:
            UartHost_WriteLine("");
            UartHost_WriteLine("Editing SERVO LPF");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST:
            UartHost_WriteLine("");
            UartHost_WriteLine("Editing minContrast");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH:
            UartHost_WriteLine("");
            UartHost_WriteLine("Editing edgeHighPct");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW:
            UartHost_WriteLine("");
            UartHost_WriteLine("Editing edgeLowPct");
            break;
        default:
            return;
    }

    if ((screen == SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST) ||
        (screen == SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH) ||
        (screen == SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW))
    {
        UartHost_WriteLine("Type digits.");
    }
    else
    {
        UartHost_WriteLine("Type digits and optional decimal point.");
    }
    UartHost_WriteString("Press ");
    UartHost_WriteChar(SERIAL_TUNE_COMMIT_CHAR);
    UartHost_WriteLine(" to save, q to cancel.");
    UartHost_WriteString("edit> ");
}

static void serial_tune_echo_char(char ch)
{
    UartHost_WriteString("RX: ");
    UartHost_WriteChar(ch);
    UartHost_WriteString("\r\n");
}

static boolean serial_tune_parse_value(const char *text, float *value)
{
    uint32 idx = 0U;
    uint32 intPart = 0U;
    uint32 fracPart = 0U;
    uint32 fracScale = 1U;
    boolean seenDigit = FALSE;
    boolean seenDot = FALSE;

    if ((text == NULL_PTR) || (value == NULL_PTR) || (text[0] == '\0'))
    {
        return FALSE;
    }

    while (text[idx] != '\0')
    {
        char ch = text[idx];

        if ((ch >= '0') && (ch <= '9'))
        {
            seenDigit = TRUE;
            if (seenDot == TRUE)
            {
                if (fracScale < 1000U)
                {
                    fracPart = (fracPart * 10U) + (uint32)(ch - '0');
                    fracScale *= 10U;
                }
            }
            else
            {
                if (intPart > 999U)
                {
                    return FALSE;
                }
                intPart = (intPart * 10U) + (uint32)(ch - '0');
            }
        }
        else if ((ch == '.') && (seenDot != TRUE))
        {
            seenDot = TRUE;
        }
        else
        {
            return FALSE;
        }

        idx++;
    }

    if (seenDigit != TRUE)
    {
        return FALSE;
    }

    *value = (float)intPart + ((float)fracPart / (float)fracScale);
    return TRUE;
}

static boolean serial_tune_parse_int_value(const char *text, sint16 *value)
{
    uint32 idx = 0U;
    sint32 out = 0;

    if ((text == NULL_PTR) || (value == NULL_PTR) || (text[0] == '\0'))
    {
        return FALSE;
    }

    while (text[idx] != '\0')
    {
        char ch = text[idx];
        if ((ch < '0') || (ch > '9'))
        {
            return FALSE;
        }

        out = (out * 10) + (sint32)(ch - '0');
        if (out > 32767)
        {
            return FALSE;
        }

        idx++;
    }

    *value = (sint16)out;
    return TRUE;
}

float *serial_tune_active_value_ptr(SerialTuneState_t *st)
{
    if (st == NULL_PTR)
    {
        return NULL_PTR;
    }

    switch (st->screen)
    {
        case SERIAL_TUNE_SCREEN_EDIT_KP:
            return &st->kp;
        case SERIAL_TUNE_SCREEN_EDIT_KI:
            return &st->ki;
        case SERIAL_TUNE_SCREEN_EDIT_KD:
            return &st->kd;
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF:
            return &st->servoLpfAlpha;
        default:
            return NULL_PTR;
    }
}

sint16 *serial_tune_active_int_ptr(SerialTuneState_t *st)
{
    if (st == NULL_PTR)
    {
        return NULL_PTR;
    }

    switch (st->screen)
    {
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP:
            return &st->servoClamp;
        default:
            return NULL_PTR;
    }
}

const char *serial_tune_active_label(const SerialTuneState_t *st)
{
    if (st == NULL_PTR)
    {
        return "";
    }

    switch (st->screen)
    {
        case SERIAL_TUNE_SCREEN_EDIT_KP:
            return "KP";
        case SERIAL_TUNE_SCREEN_EDIT_KI:
            return "KI";
        case SERIAL_TUNE_SCREEN_EDIT_KD:
            return "KD";
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP:
            return "SERVO CLAMP";
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF:
            return "SERVO LPF";
        case SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST:
            return "minContrast";
        case SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH:
            return "edgeHighPct";
        case SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW:
            return "edgeLowPct";
        default:
            return "";
    }
}

static boolean serial_tune_is_vision_screen(SerialTuneScreen_t screen)
{
    return (boolean)((screen == SERIAL_TUNE_SCREEN_VISION_MENU) ||
                     (screen == SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST) ||
                     (screen == SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH) ||
                     (screen == SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW));
}

static boolean serial_tune_requires_integer(SerialTuneScreen_t screen)
{
    return (boolean)((screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP) ||
                     (screen == SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST) ||
                     (screen == SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH) ||
                     (screen == SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW));
}

static void serial_tune_return_from_edit(SerialTuneState_t *st);

static void serial_tune_enter_pid_menu(SerialTuneState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->screen = SERIAL_TUNE_SCREEN_MENU;
    st->inputLen = 0U;
    st->inputBuf[0] = '\0';
    serial_tune_draw(st);
    StatusLed_Green();
    serial_tune_print_pid_menu();
}

static void serial_tune_enter_servo_menu(SerialTuneState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->screen = SERIAL_TUNE_SCREEN_SERVO_MENU;
    st->inputLen = 0U;
    st->inputBuf[0] = '\0';
    serial_tune_draw(st);
    StatusLed_Green();
    serial_tune_print_servo_menu();
}

static void serial_tune_enter_vision_menu(SerialTuneState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->screen = SERIAL_TUNE_SCREEN_VISION_MENU;
    st->inputLen = 0U;
    st->inputBuf[0] = '\0';
    serial_tune_draw(st);
    StatusLed_Green();
    serial_tune_print_vision_menu();
}

static void serial_tune_return_from_edit(SerialTuneState_t *st)
{
    if (st == NULL_PTR)
    {
        return;
    }

    if ((st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP) ||
        (st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF))
    {
        serial_tune_enter_servo_menu(st);
    }
    else if (serial_tune_is_vision_screen(st->screen) == TRUE)
    {
        serial_tune_enter_vision_menu(st);
    }
    else
    {
        serial_tune_enter_pid_menu(st);
    }
}

static void serial_tune_enter_edit(SerialTuneState_t *st, SerialTuneScreen_t screen)
{
    if (st == NULL_PTR)
    {
        return;
    }

    st->screen = screen;
    st->inputLen = 0U;
    st->inputBuf[0] = '\0';
    serial_tune_draw(st);
    StatusLed_Yellow();
    serial_tune_print_edit_help(screen);
}

static void serial_tune_handle_pid_menu_char(SerialTuneState_t *st, char ch)
{
    if (st == NULL_PTR)
    {
        return;
    }

    switch (ch)
    {
        case 'p':
            serial_tune_enter_edit(st, SERIAL_TUNE_SCREEN_EDIT_KP);
            break;
        case 'i':
            serial_tune_enter_edit(st, SERIAL_TUNE_SCREEN_EDIT_KI);
            break;
        case 'd':
            serial_tune_enter_edit(st, SERIAL_TUNE_SCREEN_EDIT_KD);
            break;
        case 's':
            serial_tune_enter_servo_menu(st);
            break;
        case 'v':
            serial_tune_enter_vision_menu(st);
            break;
        case 'q':
            serial_tune_print_pid_menu();
            break;
        default:
            UartHost_WriteLine("Unknown key. Press q for menu.");
            UartHost_WriteString("menu> ");
            break;
    }
}

static void serial_tune_handle_vision_menu_char(SerialTuneState_t *st, char ch)
{
    if (st == NULL_PTR)
    {
        return;
    }

    switch (ch)
    {
        case 'm':
            serial_tune_enter_edit(st, SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST);
            break;
        case 'h':
            serial_tune_enter_edit(st, SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH);
            break;
        case 'l':
            serial_tune_enter_edit(st, SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW);
            break;
        case 'p':
            serial_tune_enter_pid_menu(st);
            break;
        case 'q':
            serial_tune_print_vision_menu();
            break;
        default:
            UartHost_WriteLine("Unknown key.");
            UartHost_WriteString("vision> ");
            break;
    }
}

static void serial_tune_handle_servo_menu_char(SerialTuneState_t *st, char ch)
{
    if (st == NULL_PTR)
    {
        return;
    }

    switch (ch)
    {
        case 'c':
            serial_tune_enter_edit(st, SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP);
            break;
        case 'l':
            serial_tune_enter_edit(st, SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF);
            break;
        case 'p':
            serial_tune_enter_pid_menu(st);
            break;
        case 'q':
            serial_tune_print_servo_menu();
            break;
        default:
            UartHost_WriteLine("Unknown key.");
            UartHost_WriteString("servo> ");
            break;
    }
}

static void serial_tune_handle_edit_char(SerialTuneState_t *st, char ch)
{
    float parsedValue;
    float *activeValue;
    const char *label;
    char lineBuf[24];

    if (st == NULL_PTR)
    {
        return;
    }

    if (ch == 'q')
    {
        UartHost_WriteLine("Edit cancelled.");
        serial_tune_return_from_edit(st);
        return;
    }

    if ((ch == '\b') || ((uint8)ch == 0x7FU))
    {
        if (st->inputLen > 0U)
        {
            st->inputLen--;
            st->inputBuf[st->inputLen] = '\0';
            UartHost_WriteString("\b \b");
        }
        return;
    }

    if (ch == SERIAL_TUNE_COMMIT_CHAR)
    {
        if (serial_tune_is_vision_screen(st->screen) == TRUE)
        {
            sint16 parsedIntValue;

            if (serial_tune_parse_int_value(st->inputBuf, &parsedIntValue) != TRUE)
            {
                UartHost_WriteLine("");
                UartHost_WriteLine("Invalid number.");
                UartHost_WriteString("edit> ");
                return;
            }

            switch (st->screen)
            {
                case SERIAL_TUNE_SCREEN_EDIT_VISION_MIN_CONTRAST:
                    if (parsedIntValue > 4095)
                    {
                        UartHost_WriteLine("");
                        UartHost_WriteLine("Range: 0..4095.");
                        UartHost_WriteString("edit> ");
                        return;
                    }
                    st->visionMinContrast = (uint16)parsedIntValue;
                    g_runtimeTune.lineDetector.minContrast = st->visionMinContrast;
                    break;
                case SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_HIGH:
                    if ((parsedIntValue < 1) || (parsedIntValue > 100))
                    {
                        UartHost_WriteLine("");
                        UartHost_WriteLine("Range: 1..100.");
                        UartHost_WriteString("edit> ");
                        return;
                    }
                    st->visionEdgeHighPct = (uint8)parsedIntValue;
                    g_runtimeTune.lineDetector.edgeHighPct = st->visionEdgeHighPct;
                    break;
                case SERIAL_TUNE_SCREEN_EDIT_VISION_EDGE_LOW:
                    if ((parsedIntValue < 1) || (parsedIntValue > 100))
                    {
                        UartHost_WriteLine("");
                        UartHost_WriteLine("Range: 1..100.");
                        UartHost_WriteString("edit> ");
                        return;
                    }
                    st->visionEdgeLowPct = (uint8)parsedIntValue;
                    g_runtimeTune.lineDetector.edgeLowPct = st->visionEdgeLowPct;
                    break;
                default:
                    serial_tune_return_from_edit(st);
                    return;
            }
        }
        else if (serial_tune_active_int_ptr(st) != NULL_PTR)
        {
            sint16 parsedIntValue;

            if (serial_tune_parse_int_value(st->inputBuf, &parsedIntValue) != TRUE)
            {
                UartHost_WriteLine("");
                UartHost_WriteLine("Invalid number.");
                UartHost_WriteString("edit> ");
                return;
            }

            *serial_tune_active_int_ptr(st) = parsedIntValue;
            g_runtimeTune.profile.steerClamp = st->servoClamp;
        }
        else
        {
            if (serial_tune_parse_value(st->inputBuf, &parsedValue) != TRUE)
            {
                UartHost_WriteLine("");
                UartHost_WriteLine("Invalid number.");
                UartHost_WriteString("edit> ");
                return;
            }

            activeValue = serial_tune_active_value_ptr(st);
            if (activeValue == NULL_PTR)
            {
                serial_tune_return_from_edit(st);
                return;
            }

            *activeValue = parsedValue;

            switch (st->screen)
            {
                case SERIAL_TUNE_SCREEN_EDIT_KP:
                    g_runtimeTune.profile.kp = st->kp;
                    break;
                case SERIAL_TUNE_SCREEN_EDIT_KI:
                    g_runtimeTune.profile.ki = st->ki;
                    break;
                case SERIAL_TUNE_SCREEN_EDIT_KD:
                    g_runtimeTune.profile.kd = st->kd;
                    break;
                case SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF:
                    g_runtimeTune.profile.steerLpfAlpha = st->servoLpfAlpha;
                    break;
                default:
                    break;
            }
        }

        label = serial_tune_active_label(st);
        (void)snprintf(lineBuf, sizeof(lineBuf), "%s updated.", label);
        UartHost_WriteLine("");
        UartHost_WriteLine(lineBuf);
        serial_tune_return_from_edit(st);
        return;
    }

    if (((ch >= '0') && (ch <= '9')) || (ch == '.'))
    {
        if (st->inputLen < SERIAL_TUNE_INPUT_MAX_LEN)
        {
            if ((ch == '.') && (serial_tune_requires_integer(st->screen) == TRUE))
            {
                return;
            }

            if ((ch == '.') && (strchr(st->inputBuf, '.') != NULL_PTR))
            {
                return;
            }

            st->inputBuf[st->inputLen] = ch;
            st->inputLen++;
            st->inputBuf[st->inputLen] = '\0';
            UartHost_WriteChar(ch);
        }
        return;
    }
}
void serial_tune_test_enter(uint32 nowMs)
{
    (void)nowMs;
    RuntimeTune_InitDefaults();

    (void)memset(&g_serialTune, 0, sizeof(g_serialTune));
    g_serialTune.kp = g_runtimeTune.profile.kp;
    g_serialTune.ki = g_runtimeTune.profile.ki;
    g_serialTune.kd = g_runtimeTune.profile.kd;
    g_serialTune.servoClamp = g_runtimeTune.profile.steerClamp;
    g_serialTune.servoLpfAlpha = g_runtimeTune.profile.steerLpfAlpha;
    g_serialTune.visionMinContrast = g_runtimeTune.lineDetector.minContrast;
    g_serialTune.visionEdgeHighPct = g_runtimeTune.lineDetector.edgeHighPct;
    g_serialTune.visionEdgeLowPct = g_runtimeTune.lineDetector.edgeLowPct;
    g_serialTune.screen = SERIAL_TUNE_SCREEN_WAIT;

    serial_tune_draw(&g_serialTune);
    StatusLed_Blue();
    UartHost_WriteLine("");
    UartHost_WriteLine("=== Serial Tune Mode ===");
    UartHost_WriteLine("Open terminal at 115200 8N1.");
    UartHost_WriteString("Send '");
    UartHost_WriteChar(SERIAL_TUNE_CONNECT_CHAR);
    UartHost_WriteLine("' to connect.");
    UartHost_WriteString("> ");
}

void serial_tune_test_update(void)
{
    char rxChar;

    while (UartHost_TryReadChar(&rxChar) == TRUE)
    {
        if (g_serialTune.connected != TRUE)
        {
            if (rxChar != SERIAL_TUNE_CONNECT_CHAR)
            {
                serial_tune_echo_char(rxChar);
                UartHost_WriteString("Send '");
                UartHost_WriteChar(SERIAL_TUNE_CONNECT_CHAR);
                UartHost_WriteLine("' to connect.");
                UartHost_WriteString("> ");
                continue;
            }

            g_serialTune.connected = TRUE;
            StatusLed_Green();
            serial_tune_enter_pid_menu(&g_serialTune);
            continue;
        }

        if (g_serialTune.screen == SERIAL_TUNE_SCREEN_MENU)
        {
            serial_tune_handle_pid_menu_char(&g_serialTune, rxChar);
        }
        else if (g_serialTune.screen == SERIAL_TUNE_SCREEN_SERVO_MENU)
        {
            serial_tune_handle_servo_menu_char(&g_serialTune, rxChar);
        }
        else if (g_serialTune.screen == SERIAL_TUNE_SCREEN_VISION_MENU)
        {
            serial_tune_handle_vision_menu_char(&g_serialTune, rxChar);
        }
        else
        {
            serial_tune_handle_edit_char(&g_serialTune, rxChar);
        }
    }
}

void serial_tune_test_exit(void)
{
    UartHost_WriteLine("");
    UartHost_WriteLine("Leaving Serial Tune.");
}
