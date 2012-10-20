function item_Pause_OnSelected(self)
    
end

function item_Delete_OnSelected(self)
    local choice = self:GetOwner():GetBindHostWnd():GetUserData()
	choice.tree:DeleteItem(choice.id)
	choice.tree:SetAdjust()
end

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

function OnInitControl(self)
	local menuItem1 = self:GetOwner():GetUIObject("item.Pause")
	local menuItem2 = self:GetOwner():GetUIObject("item.Delete")
	local itemContainer1 = CreateMenuItemContainer(menuItem1)
	local itemContainer2 = CreateMenuItemContainer(menuItem2)
	menuItem1:SetContainer(itemContainer1)
	menuItem2:SetContainer(itemContainer2)
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