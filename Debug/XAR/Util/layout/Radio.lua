--lua文件必须是UTF-8编码的(最好无BOM头)
function SetState(self,newState)
    local attr = self:GetAttribute()
	
	if newState ~= attr.NowState then
		if newState == 1 then
		    local radio = self:GetControlObject("radio")
			radio:SetResID("bitmap.radio.checked")
			local text = self:GetControlObject("text")
			text:SetEffectColorResID("system.lightblue")
		elseif newState == 0 then
			local radio = self:GetControlObject("radio")
			radio:SetResID("bitmap.radio.normal")
			local text = self:GetControlObject("text")
			text:SetEffectColorResID("system.transparent")
		end
		
		attr.NowState = newState
	end
end

function GetState(self)
	local attr = self:GetAttribute()
	return attr.NowState
end

function SetText(self, txt)
	local attr = self:GetAttribute()
	attr.Text = txt
	
	local text = self:GetControlObject("text")
	text:SetText(txt)
end

function OnMouseMove(self)
    self:SetCursorID ("IDC_HAND")
end


function OnMouseLeave(self)
	self:SetCursorID ("IDC_ARROW")
end


function OnLButtonDown(self)
    local attr = self:GetAttribute()
	if attr.NowState==0 then
		self:FireExtEvent("OnClick")
		self:SetState(1)
	end
end