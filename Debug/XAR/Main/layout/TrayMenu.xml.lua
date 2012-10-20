function CreateMenuItemContainer(self,...)
	local itemContainer = {}
	itemContainer.items = {...}
	itemContainer.hoveringItem = nil
	itemContainer.enteredItem = nil
	
	itemContainer.ChangeEnteredItem = function(self2,enteredItem)
		if self2.enteredItem then
			self2.enteredItem:SetEntered(false)
		end
		if enteredItem then
			enteredItem:SetEntered(true)
		end
		self2.enteredItem = enteredItem
	end
	itemContainer.GetHoveringItem = function(self2)
		return self2.hoveringItem
	end
	itemContainer.ChangeHoveringItem = function(self2,hoveringItem)
		if hoveringItem and self2.hoveringItem then
			if hoveringItem:GetID() == self2.hoveringItem:GetID() then
				return 
			end
		end
		if hoveringItem then
			if not hoveringItem:PopupSubMenu() then
				hoveringItem = nil
			end
		end
		if self2.hoveringItem then
			self2.hoveringItem:DestroySubMenu()
		end
		self2.hoveringItem = hoveringItem
	end
	return itemContainer
end

function TrayMenu_OnInitControl(self)
	local menuItem = self:GetOwner():GetUIObject("item.Quit")
	local itemContainer = CreateMenuItemContainer(menuItem)
	menuItem:SetContainer(itemContainer)
end

function TrayMenu_item_Quit_OnSelected(self)
	local app = XLGetObject("V6Chat3App")
	app:Quit()
end

function OnPopupMenu(self)
    local menuTree = self:GetBindUIObjectTree()
	local bkg = menuTree:GetUIObject("bkg")
	
	local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")	
	local aniAlpha = aniFactory:CreateAnimation("AlphaChangeAnimation")
	aniAlpha:BindRenderObj(bkg)
	aniAlpha:SetTotalTime(250)
	aniAlpha:SetKeyFrameAlpha(0,255)
	menuTree:AddAnimation(aniAlpha)
	aniAlpha:Resume()
end