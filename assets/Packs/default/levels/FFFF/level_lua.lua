function Init()
    --Unlock 2 screen variables, y and x
	asmWrite(0, 0x1412, 1)
	asmWrite(1, 0x1411, 1)
    asmWrite(16, 0x1464, 2)

    --Write background dividers
	asmWrite(2, 0x3F06, 1)
	asmWrite(2, 0x3F07, 2)

    --Initialize Wavy HDMA
    asmWrite(0x0F02, 0x4330, 2) --Use mode 02 on register 210D
    asmWrite(0x007000, 0x4332, 3) --Set HDMA table to 0x007000 in RAM
    asmWrite(8, 0x420c, 1) --Enable HDMA Channel 3 only 

    --l3
    asmWrite(8, 0x3F1F, 1)
    --scroll
    asmWrite(4, 0x3F1C, 1)
    asmWrite(4, 0x3F1D, 1)
    asmWrite(0xF1, 0x3F1E, 1)
end

local timer = 0
function Main()
	timer = timer + 1
	asmWrite(336/2, 0x7000, 1)
	asmWrite(timer*-0.25, 0x7001, 2)
	asmWrite(-16-38+336/2, 0x7003, 1)
	asmWrite(timer*-0.25, 0x7004, 2)
	asmWrite(38, 0x7006, 1)
	asmWrite(timer*-0.125, 0x7007, 2)
    asmWrite(0, 0x7009, 1)

    asmWrite(0x10, 0x7B, 1)
    asmWrite(0xF0, 0x7D, 1)
    
    --Snow idk
    local plr = math.random(1, asmRead(0x3F0F, 1))
    local x = getPlayerX(plr) + math.random(-384, 384)
	for i=1,3 do
    	createParticle(0x76, 0x00, 0x88, 0xFF, x, 512+16, math.random() * 2, -4-math.random()*2, 0, 0, 0)
	end
end