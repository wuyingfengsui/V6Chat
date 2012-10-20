function SetChildVisible(visible)
	local hostWndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local tipsHostWnd = hostWndManager:GetHostWnd("InfoTips")
	local tipsTree = tipsHostWnd:GetBindUIObjectTree()
				
	local bkg = tipsTree:GetUIObject("bkg")
	bkg:SetVisible(visible)
	local name = tipsTree:GetUIObject("name")
	name:SetVisible(visible)
	local introduction = tipsTree:GetUIObject("introduction")
	introduction:SetVisible(visible)
	
	local head = tipsTree:GetUIObject("thumbnails")
	if head then
	    head:SetVisible(visible)
		--local sex = tipsTree:GetUIObject("sex")
		--sex:SetVisible(visible)
		
	else
	    local owner = tipsTree:GetUIObject("owner")
		owner:SetVisible(visible)
	end
end


function OnShowWindow(self, isShow)
    if isShow then
	    local data = self:GetUserData()
		if data then
		    local tipsTree = self:GetBindUIObjectTree()
		
		    local name = tipsTree:GetUIObject("name")
		    name:SetText(data.name)
		    local introduction = tipsTree:GetUIObject("introduction")
		    introduction:SetText(data.introduction)
			
		    local head = tipsTree:GetUIObject("thumbnails")
		    if head then
		        if data.head then
			        local cl, w, h = data.head:GetInfo()
					if 50 > h then
						head:SetObjPos2(10, 10, w, h)
					else
						head:SetObjPos2(10, 10, 50*w/h, 50)
					end
					head:SetBitmap(data.head)
			    end
			    --[[local sex = tipsTree:GetUIObject("sex")
			    if data.sex == 1 then
			        sex:SetText("男")
			    elseif data.sex == 2 then
			        sex:SetText("女")
			    else
			        sex:SetText("保密")
			    end
				--]]
		    else
		        local owner = tipsTree:GetUIObject("owner")
			    owner:SetText("圈主：" .. data.owner)
		    end
		end
		
		local function onAniFinish(self,oldState,newState)
		    if newState == 4 then
				SetChildVisible(true)
			end
	    end
		
		local tipsTree = self:GetBindUIObjectTree()
	    local bkg = tipsTree:GetUIObject("small_map")
	    local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")	
	    local aniAlpha = aniFactory:CreateAnimation("AlphaChangeAnimation")
	    aniAlpha:BindRenderObj(bkg)
	    aniAlpha:SetTotalTime(250)
	    aniAlpha:SetKeyFrameAlpha(0,255)
		aniAlpha:AttachListener(true,onAniFinish)
	    tipsTree:AddAnimation(aniAlpha)
	    aniAlpha:Resume()
	end
end