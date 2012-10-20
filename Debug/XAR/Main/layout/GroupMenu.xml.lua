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

function GroupMenu_OnInitControl(self)
	local menuItem1 = self:GetOwner():GetUIObject("item.Delete")
	local menuItem2 = self:GetOwner():GetUIObject("item.Add")
	local itemContainer1 = CreateMenuItemContainer(menuItem1)
	local itemContainer2 = CreateMenuItemContainer(menuItem2)
	menuItem1:SetContainer(itemContainer1)
	menuItem2:SetContainer(itemContainer2)
end

function GroupMenu_item_Delete_OnSelected(self)
	local choice = self:GetOwner():GetBindHostWnd():GetUserData()
	if #choice.item.children == 0 then
	    choice.tree:RemoveItem(choice.item)
	    for i=1,#choice.groups do
		    if choice.groups[i] == choice.item then
			    table.remove(choice.groups,i)
			    break
		    end
	    end
	else
	    XLMessageBox("分组不为空，不能删除！")
	end
end

function GroupMenu_item_Add_OnSelected(self)
    local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
	local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")

	local HostWndTemplate = templateMananger:GetTemplate("AddGroup.Wnd","HostWndTemplate")
	local HostWnd = HostWndTemplate:CreateInstance("AddGroupWnd")
	local TreeTemplate = templateMananger:GetTemplate("AddGroup.Tree","ObjectTreeTemplate")
	local Tree = TreeTemplate:CreateInstance("AddGroupTree")
	HostWnd:BindUIObjectTree(Tree)
	HostWnd:DoModal()
	
	local group_name = HostWnd:GetUserData() 
	local has_same = false
	local choice = self:GetOwner():GetBindHostWnd():GetUserData()
	for i=1,#choice.groups do
		if choice.groups[i].txt == group_name then
		    XLMessageBox("分组已存在！")
			has_same = true
			break
		end
	end
	if group_name and not has_same then
		local new_group = choice.tree:InsertItemText(1,group_name,nil,nil)
		table.insert(choice.groups, new_group)
		choice.tree:SetAdjust()
	end
	
	local objtreeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	objtreeManager:DestroyTree("AddGroupTree")
	hostwndManager:RemoveHostWnd("AddGroupWnd")
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