function AddTab(self, tabObj, icoObj, isShow)
	local attr = self:GetAttribute()
	attr.tabsNum = attr.tabsNum + 1
	if not isShow then
	    tabObj:SetVisible(false)
	    tabObj:SetChildrenVisible(false)
		icoObj:SetVisible(false)
	else
	    attr.nowTabNum = attr.tabsNum - 1
	end
	self:AddChild(tabObj)
	self:AddChild(icoObj)
end

function ShowTab(self, tabNum)
    local attr = self:GetAttribute()
	
	local tab = self:GetChildByIndex(2*tabNum + 3)
	local tabicon = self:GetChildByIndex(2*tabNum + 4)
	if tab then
		tabicon:SetVisible(true)
		tabicon:SetChildrenVisible(true)
		tab:SetVisible(true)
		tab:SetChildrenVisible(true)
	end
end

function UpdateTab(self, direction, oldtabNum, newtabNum)
    local attr = self:GetAttribute()
	attr.nowTabNum = newtabNum
	local tab = nil
	tab = self:GetChildByIndex(2*oldtabNum + 3)
	
	if tab then
	    local bmp = tab:GetWindowBitmap()
		local bkg = self:GetControlObject("bkg")
	    bkg:SetBitmap(bmp)
		
		local function onAniFinish(self,oldState,newState)
		    if newState == 4 then
				local treeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
				local main = treeManager:GetUIObjectTree("MainObjectTree")
				local tabs = main:GetUIObject("tabs")
				tabs:ShowTab(tabs:GetNowTabNum())
			end
	    end
		
		tab:SetVisible(false)
		tab:SetChildrenVisible(false)
		
		local tabicon = self:GetChildByIndex(2*oldtabNum + 4)
		tabicon:SetVisible(false)
		tabicon:SetChildrenVisible(false)
		
		local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")
        local posAni = aniFactory:CreateAnimation("PosChangeAnimation")
	    posAni:SetTotalTime(200)
	    posAni:BindLayoutObj(bkg)
		local left, top, right, bottom = tab:GetObjPos()
		left = left - 5
		right = right - 5
		top = top - 50
		if direction == 0 then
		    posAni:SetKeyFrameRect(left, top, right, bottom, left, top, left, bottom)
		else
		    posAni:SetKeyFrameRect(left, top, right, bottom, right, top, right, bottom)
		end
		local owner = self:GetOwner()
		posAni:AttachListener(true,onAniFinish)
	    owner:AddAnimation(posAni)
	    posAni:Resume()
	end
end

function GetNowTabNum(self)
    local attr = self:GetAttribute()
	return attr.nowTabNum
end


function left_btn_OnClick(self)
    local control = self:GetOwnerControl()
	local attr = control:GetAttribute()
	local tabNum = attr.nowTabNum - 1
	if tabNum<0 then
	    tabNum = attr.tabsNum - 1
	end
	control:UpdateTab(0, attr.nowTabNum, tabNum)
end

function right_btn_OnClick(self)
    local control = self:GetOwnerControl()
	local attr = control:GetAttribute()
	local tabNum = attr.nowTabNum + 1
	if tabNum>attr.tabsNum - 1 then
	    tabNum = 0
	end
	
	control:UpdateTab(1, attr.nowTabNum, tabNum)
end

function OnBind(self)
    local attr = self:GetAttribute()
	attr.tabsNum = 0
	if attr.background then
	    local bkg = self:GetControlObject("bkg")
	    bkg:SetTextureID(attr.background)
	end
end