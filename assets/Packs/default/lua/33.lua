--Fireball
function Init(spr_num)
    asmWrite(4, 0x2A80 + spr_num, 1) --Destroy when offscreen
end
local oam_table = {
    0x2C, 0x2A,
    0x2D, 0x2A,
    0x2C, 0x1A,
    0x2D, 0x1A
}

local timer = 0
function Main(spr_num)
    timer = timer + 1
    --speed
    asmWrite(asmRead(0x2680 + spr_num, 1)*-16, 0x2400 + spr_num, 1)

    --set flags
    asmWrite(0xC0, 0x2600 + spr_num, 1)
    asmWrite(0x11, 0x2880 + spr_num, 1)

    --set size
    asmWrite(8, 0x2500 + spr_num, 1)
    asmWrite(8, 0x2580 + spr_num, 1)
    
    local t = math.floor(timer / 4) % 4
    --pushOAM (nil or sprite_index, offset_x, offset_y, tile, size, properties, angle, scalex, scaley) 
    pushOAM(
        spr_num,
        0,-24,
        oam_table[1 + t * 2], 0,
        oam_table[2 + t * 2]
    )

    asmWrite(asmRead(0x2680 + spr_num, 1)*-0x28, 0x2400 + spr_num, 1)

    if asmRead(0x2480 + spr_num, 1) > 0x7F then
        asmWrite(math.max(0xD0, asmRead(0x2480 + spr_num, 1) - 4), 0x2480 + spr_num, 1)
    else
        asmWrite(asmRead(0x2480 + spr_num, 1) - 4, 0x2480 + spr_num, 1)
    end
    if asmCheckBit(0x2780 + spr_num, 1) then --Has collided vertically
        if asmRead(0x2480 + spr_num, 1) > 0x7F then
            asmWrite(0x38, 0x2480 + spr_num, 1)
        else
            asmWrite(0x00, 0x2480 + spr_num, 1)
        end
    end
    if asmCheckBit(0x2780 + spr_num, 0) or asmRead(0x2B00 + spr_num, 1) == 1 then --Has collided horizontally
        createParticle(0x60, 0x11, 0x88, 4, getSpriteX(spr_num)-4, getSpriteY(spr_num)-4, 0, 0, 0, 0, 0)
        asmWrite(0, 0x2000 + spr_num, 1)
        asmWrite(1, 0x1DF9, 1)
    end
end