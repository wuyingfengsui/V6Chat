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

function GroupsListMenu_OnInitControl(self)
	local menuItem1 = self:GetOwner():GetUIObject("item.Chat")
	local menuItem2 = self:GetOwner():GetUIObject("item.Quit")
	local menuItem3 = self:GetOwner():GetUIObject("item.Move")
	local itemContainer1 = CreateMenuItemContainer(menuItem1)
	menuItem1:SetContainer(itemContainer1)
	local itemContainer2 = CreateMenuItemContainer(menuItem2)
	menuItem2:SetContainer(itemContainer2)
	local itemContainer3 = CreateMenuItemContainer(menuItem3)
	menuItem3:SetContainer(itemContainer3)
end

function GroupsListMenu_item_Chat_OnSelected(self)
    local choice = self:GetOwner():GetBindHostWnd():GetUserData()
	XLMessageBox(choice.item.id)
end

function GroupsListMenu_item_Quit_OnSelected(self)
    local choice = self:GetOwner():GetBindHostWnd():GetUserData()
	choice.tree:RemoveItem(choice.item)
	choice.item = nil
end

function GroupsListMenu_SubMenu_GroupGroups_Item_OnSelected(self)
    local choice = self:GetOwner():GetBindHostWnd():GetUserData()
	choice.tree:RemoveItem(choice.item)
	for i=1,#choice.groups do
		if choice.groups[i].txt == self:GetText() then
		    choice.tree:InsertItemText(choice.item.id, choice.item.txt, choice.item.ext_txt, choice.groups[i])
			choice.tree:SetAdjust(true)
		end
	end
	choice.item = nil
end

function GroupsListMenu_SubMenu_GroupGroups_OnBind(self)
    local choice = self:GetOwner():GetBindHostWnd():GetUserData()
	local recentUrls = choice.groups
	--recentUrls = {"xunlei.com", "163.com"}
	local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")	
	local bkgObj = self:GetChildByIndex(0)
	local itemTop = 0
	local urlCnt = table.maxn(recentUrls)
	local blankItem = bkgObj:GetChildByIndex(0)
	local itemContainer = CreateMenuItemContainer()
	if urlCnt ~= 0 then
		bkgObj:RemoveChild(blankItem)
		local itemTemplate = templateMananger:GetTemplate("GroupsListMenu.SubMenu.GroupGroups.Item","ObjectTemplate")
		--XLLog("renct urls has "..urlCnt)
		for i=1, urlCnt do
			local itemObj = itemTemplate:CreateInstance("GroupGroups.item."..i)
			itemObj:SetContainer(itemContainer)
			itemTop = 20 * (i - 1)
			itemObj:SetObjPos("0", tostring(itemTop) , "father.width", tostring(itemTop + 20))
			itemObj:SetText(recentUrls[i].txt)
			bkgObj:AddChild(itemObj)
		end
	else
		blankItem:SetContainer(itemContainer)
	end
	itemTop = itemTop + 30
	bkgObj:SetObjPos("0","0","father.width",tostring(itemTop))
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