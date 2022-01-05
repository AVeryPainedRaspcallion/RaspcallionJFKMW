--Fire Flower
local rise_t = 0
local time = 0
function Init(spr_num)
    asmWrite(0x07, 0x2000 + spr_num, 1)
end
function Main(spr_num)
    time = time + 1
    --set size
    asmWrite(16, 0x2500 + spr_num, 1)
    asmWrite(16, 0x2580 + spr_num, 1)

    if asmRead(0x2A00 + spr_num, 1) == 2 then
        rise_t = rise_t + 0.25
        asmWrite(0, 0x2600 + spr_num, 1)
        asmWrite(4, 0x2480 + spr_num, 1)
        asmWrite(0, 0x2400 + spr_num, 1)
        if rise_t > 14 then
            asmWrite(0, 0x2A00 + spr_num, 1)
        end    
        if rise_t > 1 then
            pushOAM(spr_num, 0, -16, 0x68, 0x11, 0x20C)
        end
    else
        if asmRead(0x2A00 + spr_num, 1) == 1 then
            --Fall Mode
            --Flags
            asmWrite(0, 0x2600 + spr_num, 1)
            asmWrite(0, 0x2400 + spr_num, 1)
            asmWrite(-16, 0x2480 + spr_num, 1)

            if (time % 16) < 12 then
                pushOAM(spr_num, 0, -16, 0x68, 0x11, 0xC)
            end
        else
            asmWrite(0x60, 0x2600 + spr_num, 1)
            asmWrite(0, 0x2400 + spr_num, 1)
            pushOAM(spr_num, 0, -16, 0x68, 0x11, 0xC)
        end
    end
end