#include "app_internal.h"

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
    SerialDebug_WriteLine("");
    SerialDebug_WriteLine("=== PID Tune Menu ===");
    SerialDebug_WriteLine("p - edit KP");
    SerialDebug_WriteLine("i - edit KI");
    SerialDebug_WriteLine("d - edit KD");
    SerialDebug_WriteLine("s - servo menu");
    SerialDebug_WriteString("menu> ");
}

static void serial_tune_print_servo_menu(void)
{
    SerialDebug_WriteLine("");
    SerialDebug_WriteLine("=== Servo Tune Menu ===");
    SerialDebug_WriteLine("c - edit clamp");
    SerialDebug_WriteLine("l - edit LPF");
    SerialDebug_WriteLine("p - PID menu");
    SerialDebug_WriteString("servo> ");
}

static void serial_tune_print_edit_help(SerialTuneScreen_t screen)
{
    switch (screen)
    {
        case SERIAL_TUNE_SCREEN_EDIT_KP:
            SerialDebug_WriteLine("");
            SerialDebug_WriteLine("Editing KP");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_KI:
            SerialDebug_WriteLine("");
            SerialDebug_WriteLine("Editing KI");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_KD:
            SerialDebug_WriteLine("");
            SerialDebug_WriteLine("Editing KD");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP:
            SerialDebug_WriteLine("");
            SerialDebug_WriteLine("Editing SERVO CLAMP");
            break;
        case SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF:
            SerialDebug_WriteLine("");
            SerialDebug_WriteLine("Editing SERVO LPF");
            break;
        default:
            return;
    }

    SerialDebug_WriteLine("Type digits and optional decimal point.");
    SerialDebug_WriteString("Press ");
    SerialDebug_WriteChar(SERIAL_TUNE_COMMIT_CHAR);
    SerialDebug_WriteLine(" to save, q to cancel.");
    SerialDebug_WriteString("edit> ");
}

static void serial_tune_echo_char(char ch)
{
    SerialDebug_WriteString("RX: ");
    SerialDebug_WriteChar(ch);
    SerialDebug_WriteString("\r\n");
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
        default:
            return "";
    }
}

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
        case 'q':
            serial_tune_print_pid_menu();
            break;
        default:
            SerialDebug_WriteLine("Unknown key. Press q for menu.");
            SerialDebug_WriteString("menu> ");
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
            SerialDebug_WriteLine("Unknown key.");
            SerialDebug_WriteString("servo> ");
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
        SerialDebug_WriteLine("Edit cancelled.");
        if ((st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP) ||
            (st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF))
        {
            serial_tune_enter_servo_menu(st);
        }
        else
        {
            serial_tune_enter_pid_menu(st);
        }
        return;
    }

    if ((ch == '\b') || ((uint8)ch == 0x7FU))
    {
        if (st->inputLen > 0U)
        {
            st->inputLen--;
            st->inputBuf[st->inputLen] = '\0';
            SerialDebug_WriteString("\b \b");
        }
        return;
    }

    if (ch == SERIAL_TUNE_COMMIT_CHAR)
    {
        if (serial_tune_active_int_ptr(st) != NULL_PTR)
        {
            sint16 parsedIntValue;

            if (serial_tune_parse_int_value(st->inputBuf, &parsedIntValue) != TRUE)
            {
                SerialDebug_WriteLine("");
                SerialDebug_WriteLine("Invalid number.");
                SerialDebug_WriteString("edit> ");
                return;
            }

            *serial_tune_active_int_ptr(st) = parsedIntValue;
            g_runtimeTune.profile.steerClamp = st->servoClamp;
        }
        else
        {
            if (serial_tune_parse_value(st->inputBuf, &parsedValue) != TRUE)
            {
                SerialDebug_WriteLine("");
                SerialDebug_WriteLine("Invalid number.");
                SerialDebug_WriteString("edit> ");
                return;
            }

            activeValue = serial_tune_active_value_ptr(st);
            if (activeValue == NULL_PTR)
            {
                if ((st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP) ||
                    (st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF))
                {
                    serial_tune_enter_servo_menu(st);
                }
                else
                {
                    serial_tune_enter_pid_menu(st);
                }
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
        SerialDebug_WriteLine("");
        SerialDebug_WriteLine(lineBuf);
        if ((st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_CLAMP) ||
            (st->screen == SERIAL_TUNE_SCREEN_EDIT_SERVO_LPF))
        {
            serial_tune_enter_servo_menu(st);
        }
        else
        {
            serial_tune_enter_pid_menu(st);
        }
        return;
    }

    if (((ch >= '0') && (ch <= '9')) || (ch == '.'))
    {
        if (st->inputLen < SERIAL_TUNE_INPUT_MAX_LEN)
        {
            if ((ch == '.') && (serial_tune_active_int_ptr(st) != NULL_PTR))
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
            SerialDebug_WriteChar(ch);
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
    g_serialTune.screen = SERIAL_TUNE_SCREEN_WAIT;

    serial_tune_draw(&g_serialTune);
    StatusLed_Blue();
    SerialDebug_WriteLine("");
    SerialDebug_WriteLine("=== Serial Tune Mode ===");
    SerialDebug_WriteLine("Open terminal at 115200 8N1.");
    SerialDebug_WriteString("Send '");
    SerialDebug_WriteChar(SERIAL_TUNE_CONNECT_CHAR);
    SerialDebug_WriteLine("' to connect.");
    SerialDebug_WriteString("> ");
}

void serial_tune_test_update(void)
{
    char rxChar;

    while (SerialDebug_TryReadChar(&rxChar) == TRUE)
    {
        if (g_serialTune.connected != TRUE)
        {
            if (rxChar != SERIAL_TUNE_CONNECT_CHAR)
            {
                serial_tune_echo_char(rxChar);
                SerialDebug_WriteString("Send '");
                SerialDebug_WriteChar(SERIAL_TUNE_CONNECT_CHAR);
                SerialDebug_WriteLine("' to connect.");
                SerialDebug_WriteString("> ");
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
        else
        {
            serial_tune_handle_edit_char(&g_serialTune, rxChar);
        }
    }
}

void serial_tune_test_exit(void)
{
    SerialDebug_WriteLine("");
    SerialDebug_WriteLine("Leaving Serial Tune.");
}
