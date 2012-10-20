--lua文件必须是UTF-8编码的(最好无BOM头)
function SetOriginalLeft(self, OriginalLeft)
	local attr = self:GetAttribute()
	
	local l, t, r, b = self:GetObjPos()
	self:SetObjPos(OriginalLeft, t, r + OriginalLeft - attr.OriginalLeft, b)
	
	attr.OriginalLeft = OriginalLeft
end

function SetState(self,newState)
    local attr = self:GetAttribute()
	
	if attr and newState ~= attr.NowState then
	    local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")
        local posAni = aniFactory:CreateAnimation("PosChangeAnimation")
	    posAni:SetTotalTime(attr.FloatValue*25)
	    posAni:BindLayoutObj(self)
		
		if newState == 2 then
			posAni:SetKeyFramePos(attr.OriginalLeft,attr.OriginalTop+attr.FloatValue,attr.OriginalLeft,attr.OriginalTop)
			local owner = self:GetOwner()
	        owner:AddAnimation(posAni)
	        posAni:Resume()
		elseif newState == 1 then
		    
		elseif newState == 0 then
			posAni:SetKeyFramePos(attr.OriginalLeft,attr.OriginalTop,attr.OriginalLeft,attr.OriginalTop+attr.FloatValue)
			local owner = self:GetOwner()
	        owner:AddAnimation(posAni)
	        posAni:Resume()
		end
		
		attr.NowState = newState
	end
end


function SetBkgID(self, bkgID)
    local attr = self:GetAttribute()
	attr.bkgID = bkgID
	local bkg = self:GetControlObject("bkg")
	bkg:SetResID(bkgID)
end


function SetFloatValue(self, floatValue)
    local attr = self:GetAttribute()
	attr.FloatValue = floatValue
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
	attr.OriginalLeft,attr.OriginalTop = self:GetObjPos()
	attr.OriginalTop = attr.OriginalTop - attr.FloatValue
	local bkg = self:GetControlObject("bkg")
	bkg:SetResID(attr.BkgID)
end