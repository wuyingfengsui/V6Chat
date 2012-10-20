--lua文件必须是UTF-8编码的(最好无BOM头)
function SetState(self,newState)
    local attr = self:GetAttribute()
	
	if newState ~= attr.NowState then
	    if newState == 2 then
			local frame = self:GetControlObject("frame")
			frame:SetResID("app.userimage.frame.highlight")
			self:SetCursorID ("IDC_HAND")
		elseif newState == 1 then
		    
		elseif newState == 0 then
			local frame = self:GetControlObject("frame")
			frame:SetResID("app.userimage.frame")
			self:SetCursorID ("IDC_ARROW")
		end
		
		attr.NowState = newState
	end
end

function SetImagePos(self, width, height)
	local my_image = self:GetControlObject("my_image")
	local frame = self:GetControlObject("frame")
	frame:SetObjPos2(0, 0, width + 10, height + 10)
	my_image:SetObjPos2(5, 5, width, height)
end

function SetHeadImage(self,bitmap)
	local my_image = self:GetControlObject("my_image")
	
	if bitmap then
		my_image:SetBitmap(bitmap)
	else
		local attr = self:GetAttribute()
		my_image:SetResID(attr.HeadImageID)
	end
end

function OnMouseMove(self)
    local attr = self:GetAttribute()
	if attr.NowState==0 then
	    self:SetState(2)
    end
end


function OnMouseLeave(self)
    self:SetState(0)
end


function OnLButtonDown(self)
    self:SetState(1)
end


function OnLButtonUp(self)
    local attr = self:GetAttribute()
	if attr.Enable then
		if attr.NowState==1 then
			self:FireExtEvent("OnClick")
			self:SetState(0)
		end
		
		self:SetCaptureMouse(false)
	end
end


function OnBind(self)
	local attr = self:GetAttribute()
	attr.NowState=0
	
	local my_image = self:GetControlObject("my_image")
	my_image:SetResID(attr.HeadImageID)
end