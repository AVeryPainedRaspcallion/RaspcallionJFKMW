--Star
local time = 0
local rise_t = 0
function Init(spr_num)
    asmWrite(0x08, 0x2000 + spr_num, 1)
end
function Main(spr_num)
    asmWrite(0x8, 0x2880 + spr_num, 1) --Lowgrav

    --set size
    asmWrite(16, 0x2500 + spr_num, 1)
    asmWrite(16, 0x2580 + spr_num, 1)

    --Flags
    if asmRead(0x2A00 + spr_num, 1) == 2 then
        rise_t = rise_t + 0.25
        asmWrite(0, 0x2600 + spr_num, 1)
        asmWrite(4, 0x2480 + spr_num, 1)
        asmWrite(0, 0x2400 + spr_num, 1)
        if rise_t > 14 then
            asmWrite(0, 0x2A00 + spr_num, 1)
        end
    else
        asmWrite(0x60, 0x2600 + spr_num, 1)
        if asmCheckBit(0x2780 + spr_num, 0) then --Has collided horizontally
            asmWrite(asmRead(0x2680 + spr_num, 1)*-1, 0x2680 + spr_num, 1)
        end
        if asmCheckBit(0x2780 + spr_num, 1) then --Has collided vertically
            if asmRead(0x2480 + spr_num, 1) > 0x7F then
                asmWrite(0x38, 0x2480 + spr_num, 1)
            else
                asmWrite(0x0, 0x2480 + spr_num, 1)
            end
        end
        asmWrite(asmRead(0x2680 + spr_num, 1) * -12, 0x2400 + spr_num, 1)
    end
    time = time + 1
    drawOam(
        spr_num,
        0x8E,
        0x11,
        0x00,
        0,-16,
        0x8 + ((math.floor(time / 2) % 8) + (asmRead(0x2A00 + spr_num, 1) == 2 and 0x100 or 0)
    )
end