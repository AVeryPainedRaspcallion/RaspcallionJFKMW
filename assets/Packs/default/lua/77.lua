--Cape powerup
local time = 0
local XSPDTable = {32, -32}
local YSPDTable = {0xA, 0xF6, 0x8}
local XSPD = 0
local TARGET_XSPD = -32
local TA_INDEX = 1
local YSPD = 0
local G_UP = true
function Init(spr_num)
    asmWrite(0x06, 0x2000 + spr_num, 1)
    if asmRead(0x2A00 + spr_num, 1) == 2 then
        YSPD = 48
    end
end
function Main(spr_num)
    --set size
    asmWrite(16, 0x2500 + spr_num, 1)
    asmWrite(16, 0x2580 + spr_num, 1)
    if asmRead(0x2A00 + spr_num, 1) == 2 then
        --flying mode
        if YSPD > 0 and G_UP then
            YSPD = YSPD - 1
        else
            G_UP = false
            TARGET_XSPD = XSPDTable[TA_INDEX]
            if TARGET_XSPD < 0 then
                XSPD = math.max(XSPD - 2, TARGET_XSPD)
            end
            if TARGET_XSPD > 0 then
                XSPD = math.min(XSPD + 2, TARGET_XSPD)
            end
            if XSPD == TARGET_XSPD then
                YSPD = 0
                TA_INDEX = TA_INDEX + 1
                if TA_INDEX > #XSPDTable then
                    TA_INDEX = 1
                end
            else
                local I = TARGET_XSPD < 0 and 1 or 2
                if XSPD < 0 then
                    I = I + 1
                end
                local newYS = YSPDTable[I] - 6
                YSPD = newYS
            end
        end
        asmWrite(XSPD, 0x2400 + spr_num, 1)
        asmWrite(YSPD, 0x2480 + spr_num, 1)
        asmWrite(0, 0x2600 + spr_num, 1)
        drawOam(
            spr_num,
            0x80,
            0x11,
            0x00,
            0,-16,
            0xA + (TARGET_XSPD > 0 and 0x20 or 0)
        )
    else
        if asmRead(0x2A00 + spr_num, 1) == 1 then
            --Fall Mode
            --Flags
            asmWrite(0, 0x2600 + spr_num, 1)
            asmWrite(0, 0x2400 + spr_num, 1)
            asmWrite(-16, 0x2480 + spr_num, 1)
            time = time + 1
            if (time % 16) < 12 then
                drawOam(
                    spr_num,
                    0x80,
                    0x11,
                    0x00,
                    0,-16,
                    0xA
                )
            end
        else
            --stationary mode
            asmWrite(0x60, 0x2600 + spr_num, 1)
            asmWrite(0, 0x2400 + spr_num, 1)

            drawOam(
                spr_num,
                0x80,
                0x11,
                0x00,
                0,-16,
                0xA
            )
        end
    end
end