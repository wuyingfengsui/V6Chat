--lua文件必须是UTF-8编码的(最好无BOM头)
function SetState(self,newState) 		
		local attr = self:GetAttribute()
		
		if attr then
		    if newState ~= attr.NowState then
			    local ownerTree = self:GetOwner()
			    local oldBkg = self:GetControlObject("oldBkg")
			    local bkg = self:GetControlObject("bkg")
			
			    oldBkg:SetTextureID(bkg:GetTextureID())
			    oldBkg:SetAlpha(255)
			    if newState == 0 then
				    bkg:SetTextureID(attr.NormalBkgID)
			    elseif newState == 1 then
				    bkg:SetTextureID(attr.DownBkgID)
			    elseif newState == 2 then
				    bkg:SetTextureID(attr.DisableBkgID)
			    elseif newState == 3 then
				    bkg:SetTextureID(attr.HoverBkgID)
			    end

			
			    local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")	
			    local aniAlpha = aniFactory:CreateAnimation("AlphaChangeAnimation")
			    aniAlpha:BindRenderObj(oldBkg)
			    aniAlpha:SetTotalTime(500)
			    aniAlpha:SetKeyFrameAlpha(255,0)
			    ownerTree:AddAnimation(aniAlpha)
			    aniAlpha:Resume()
			    attr.NowState = newState
		    end
		end
end

function SetText(self,newText)
	local textObj = self:GetControlObject("text")
	textObj:SetText(newText)
	
	local attr = self:GetAttribute()
	textObj:SetTextFontResID(attr.FontID)
end

function SetEnable(self, enable)
    local attr = self:GetAttribute()
	attr.Enable = enable
	
	if enable then
	    local bkg = self:GetControlObject("bkg")
	    bkg:SetTextureID(attr.NormalBkgID)
	else
	    local bkg = self:GetControlObject("bkg")
	    bkg:SetTextureID(attr.DisableBkgID)
	end
end

function GetShouldUp(self)
	local attr = self:GetAttribute()
	return attr.ShouldUp
end

function GetHasDownClick(self)
	local attr = self:GetAttribute()
	return attr.hasDownClick
end

function OnLButtonDown(self)
	local attr = self:GetAttribute()
	if attr.Enable then
		if attr.NowState==3 then
			self:SetState(1)
		elseif attr.NowState==1 then
			attr.ShouldUp = true
		end
		
		self:SetCaptureMouse(true)
	end
end

function OnLButtonUp(self)
	local attr = self:GetAttribute()
	if attr.Enable then
		if attr.NowState==1 then
			if attr.ShouldUp then
				attr.hasDownClick = false
				self:FireExtEvent("OnUpClick")
				self:SetState(0)
				attr.ShouldUp = false
			else
				attr.hasDownClick = true
				self:FireExtEvent("OnDownClick")
			end
		end
		
		self:SetCaptureMouse(false)
	end
end

function OnMouseMove(self)
	local attr = self:GetAttribute()
	if attr.Enable then
		if attr.NowState==0 then
			self:SetState(3)
		end
	end
end

function OnMouseLeave(self)
	local attr = self:GetAttribute()
	if attr.NowState ~= 1 then
		self:SetState(0)
	end
end

function OnBind(self)
	local attr = self:GetAttribute()
	self:SetText(attr.Text)
	attr.NowState=0
	local bkg = self:GetControlObject("bkg")
	bkg:SetTextureID(attr.NormalBkgID)
end