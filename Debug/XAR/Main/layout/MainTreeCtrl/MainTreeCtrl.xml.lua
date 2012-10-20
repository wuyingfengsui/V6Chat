function InsertItem(self,item, parent)
	table.insert(parent.children, item)
	return item
end

function InsertItemText(self,id,txt,ext_txt,parent)
	if parent == nil then
		local attr = self:GetAttribute()
		parent = attr.root
	end
	item = {}
	item.id = id
	item.txt = txt
	item.ext_txt = ext_txt
	item.children = {}
	item.parent = parent
	item.level = parent.level + 1
	item.visible = false	
	item.left = 0
	item.top = 0
	item.expand = false
	item.select = false
	item.hasHead = false
	item.tree = self
	return self:InsertItem(item, parent)
end

function InsertItemImageText(self,id,image,txt,ext_txt,parent)
    if parent == nil then
		local attr = self:GetAttribute()
		parent = attr.root
	end
	item = {}
	item.id = id
	item.txt = txt
	item.ext_txt = ext_txt
	item.children = {}
	item.parent = parent
	item.level = parent.level + 1
	item.visible = false	
	item.left = 0
	item.top = 0
	item.expand = false
	item.select = false
	item.hasHead = true
	item.tree = self
	item.image = image
	return self:InsertItem(item, parent)
end

function OnBind(self)
	local attr = self:GetAttribute()
	local left,top,right,bottom = self:GetObjPos()
	m_itemheight = 40
	attr.root = {}
	attr.root.children = {}
	attr.root.level = 0
	attr.m_top = 1
	attr.m_left = 0
	attr.VisibleItems = {}
	attr.maxwidth = 0
end

function RecAdjust(item, left)
	local tree = item.tree
	local attr = tree:GetAttribute()
	mapTopItem[attr.m_top] = item
	item.left = left
	item.top = attr.m_top
	attr.m_top = attr.m_top + 1
	testTextLength:SetText(item.txt)
	attr.maxwidth = math.max(attr.maxwidth, left * 12 + testTextLength:GetTextExtent())
	if item.expand then
		for i=1,#item.children do
			RecAdjust(item.children[i],left+1)
		end
	end
end

function SetAdjust(self,bVscroll)
	local attr = self:GetAttribute()
	local root = attr.root
	mapTopItem = {}
	attr.mapTopItem = mapTopItem
	--attr.maxwidth = 0
	
	if attr.m_top > 1 then attr.m_top = 1 end
	local tmp = attr.m_top
	
	for i=1,#root.children do
		RecAdjust(root.children[i], -attr.m_left)
	end
	
	for i=1,ItemVisibleCount do
		attr.VisibleItems[i]:BindAttr(mapTopItem[i])
	end
	
	totalVisible = attr.m_top - tmp
	attr.m_top = tmp
	--self:SetVScrollAdjust(bVscroll)
	--self:SetHScrollAdjust()
	--XLMessageBox(""..attr.maxwidth)
	
	if not bVscroll then
		self:UpdateVScroll()
		local vscroll = self:GetControlObject("treectrl.vscroll")
		vscroll:SetScrollPos(-(attr.m_top-1)*40, true)
	end
end

function SetHScrollAdjust(self)
	local attr = self:GetAttribute()
	local content = self:GetControlObject("Item.Cotent")
	local left,top,right,bottom = content:GetObjPos()
	local width = right - left
	local hscroll = self:GetControlObject("treectrl.hscroll")
	if width >= attr.maxwidth then
		hscroll:Show(false)
	else
		hscroll:Show(true)
	end
end

function GetRootItem(self)
	local attr = self:GetAttribute()
	return attr.root
end

function GetChildrenItemCount(self, item)
	return #item.children
end

function GetChildItemByIndex(self,item,index)
	return item.children[index]
end

function GetChildItemByTxt(self,item,txt)
    for i=1,#item.children do
		if item.children[i].txt == txt then
		    return item.children[i]
		end
	end
	return nil
end

function GetChildItemById(self,item,id)
    for i=1,#item.children do
		if item.children[i].id == id then
		    return item.children[i]
		end
	end
	return nil
end

function UpdateChildItem(self,id,image,txt,ext_txt,father_name)
	local friend = self:GetChildItemById(self:GetChildItemByTxt(self:GetRootItem(), father_name), id)
	if friend then
	    friend.id = id
	    friend.image = image
	    friend.txt = txt
	    friend.ext_txt = ext_txt
	end
end

function OffChildItem(self,id,ext_txt,father_name)
    local friend = self:GetChildItemById(self:GetChildItemByTxt(self:GetRootItem(), father_name), id)
	if friend then
	    friend.ext_txt = ext_txt
	end
end

function OnInitControl(self)
	local attr = self:GetAttribute()
	local content = self:GetControlObject("Item.Cotent")
	local left,top,right,bottom = self:GetObjPos()
	local height = bottom - top
	ItemVisibleCount = math.ceil(height / m_itemheight)
	attr.ItemVisibleCount = ItemVisibleCount
	for i=1,ItemVisibleCount do
		local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
		local obj = objFactory:CreateUIObject("item."..i, "MainTreeItem")
		content:AddChild(obj)
		table.insert(attr.VisibleItems, obj)
	end	
	local vscroll = self:GetControlObject("treectrl.vscroll")
	--local hscroll = self:GetControlObject("treectrl.hscroll")
	vscroll:Show(false)
	--hscroll:Show(false)
	
	testTextLength = self:GetControlObject("ItemTestText")
end

function GetSelected(self)
	return self:GetAttribute().select
end

function SetSelected(self,item,update)
	local attr = self:GetAttribute()
	if attr.select == item then return end
	if attr.select then
		attr.select.select = false
	end
	item.select = true
	attr.select = item
	if update then self:SetAdjust() end
end

function RemoveItem(self, item)
	--XLMessageBox(string.format("RemoveItem item.txt=%s",item.txt))
	if item.select then self:GetAttribute().select = nil end
	for i=1,#item.parent.children do
		if item.parent.children[i] == item then
			table.remove(item.parent.children,i)
			self:SetAdjust()
			break
		end
	end
end

function TreeCtrl_OnVScroll(self, fun, type, newpos)
	local owner = self:GetOwnerControl()
	local attr = owner:GetAttribute()
	
	if type ~= 3 then
	    local pos = nil
		if type == 4 then
		    pos = newpos
		else
	        pos = self:GetScrollPos()
		end
		
		if pos == 0 then
		    attr.m_top = 1
		else
		    attr.m_top = math.ceil(-pos / 40)
		end
		owner:SetAdjust(true)
		if type == 1 then
		    self:SetScrollPos( pos - 15, true )
		elseif type == 2 then
		    self:SetScrollPos( pos + 15, true )
		end
	end
end

function SetText(self,value)
	local control = self:GetOwnerControl()
	local txt = control:GetControlObject("ItemText")
	txt:SetText(value)
end

function BindAttr(self, itemattr)
	if itemattr == nil then 
		self:SetObjPos(0,0,0,0)
		return
	end
	
	local attr = self:GetAttribute()
	
	itemattr.obj = self
	local tree = self:GetOwnerControl()
	local content = tree:GetControlObject("Item.Cotent")
	local left,top,right,bottom = content:GetObjPos()
	local width = right - left
	local attr = self:GetAttribute()
	self:SetObjPos(itemattr.left*12,itemattr.top*m_itemheight-m_itemheight,width-12,itemattr.top*m_itemheight)
	
	local txt = self:GetControlObject("ItemText")
	txt:SetText(itemattr.txt)
	--local len = txt:GetTextExtent()
	local txt_left,txt_top,txt_right,txt_bottom = txt:GetObjPos()
	
	local ext_txt = self:GetControlObject("ItemExtText")
	if itemattr.ext_txt then
	    ext_txt:SetText(itemattr.ext_txt)
		ext_txt:SetVisible(true)
	else
	    ext_txt:SetVisible(false)
	end
	local ext_txt_left,ext_txt_top,ext_txt_right,ext_txt_bottom
	if ext_txt then
	    ext_txt_left,ext_txt_top,ext_txt_right,ext_txt_bottom = ext_txt:GetObjPos()
	end
	
	local img = self:GetControlObject("HeadImage")
	if itemattr.hasHead then
	    txt:SetObjPos(90,txt_top,90 + txt_right - txt_left,txt_bottom)
		if ext_txt then
		    ext_txt:SetObjPos(90,ext_txt_top,90 + ext_txt_right - ext_txt_left,ext_txt_bottom)
		end
		
		img:SetVisible(true)
		
		if itemattr.image then
		    local cl, w, h = itemattr.image:GetInfo()
			if 35 > h then
				img:SetObjPos2(15, 8, w, h)
			else
				img:SetObjPos2(15, 8, 35*w/h, 35)
			end
			img:SetBitmap(itemattr.image)
		else
		    img:SetObjPos2(15, 8, 28, 35)
			img:SetResID("app.unknown_photo")
		end
	else
	    txt:SetObjPos(15,txt_top,15 + txt_right - txt_left,txt_bottom)
		if ext_txt then
		    ext_txt:SetObjPos(15,ext_txt_top,15 + ext_txt_right - ext_txt_left,ext_txt_bottom)
		end
		
		img:SetVisible(false)
	end
	
	local expand = self:GetControlObject("Expand")
	if #itemattr.children > 0 then
		expand:SetVisible(true)
		if itemattr.expand then
			expand:SetResID("bitmap.categorytree.expand")
		else
			expand:SetResID("bitmap.categorytree.collapse")
		end
	else
		expand:SetVisible(false)
	end
	
	local select = self:GetControlObject("ItemSelect")
	select:SetVisible(itemattr.select)
	
	attr.itemattr = itemattr
end

function Item_OnMouseMove(self,x,y)
    local tree = self:GetOwnerControl()
	local attr = self:GetAttribute()
	local tree_attr = tree:GetAttribute()
	local itemattr = attr.itemattr
	if attr.itemattr.parent ~= tree_attr.root then
	    local txt = self:GetControlObject("ItemText")
		txt:SetEffectColorResID("main.tip.name.green")
	end
	
	if x>=0 and x<=15 and y>=0 and y<=40 then
	else
		tree:SetSelected(itemattr, true)
	end
end

function Item_OnLButtonDown(self,x,y)
	local tree = self:GetOwnerControl()
	local attr = self:GetAttribute()
	local itemattr = attr.itemattr
	if x>=0 and x<=15 and y>=0 and y<=40 then
		itemattr.expand = not itemattr.expand
	end
	tree:SetAdjust()
end

function Item_OnRButtonDown(self,x,y)
    local tree = self:GetOwnerControl()
	local attr = self:GetAttribute()
	local tree_attr = tree:GetAttribute()
	local itemattr = attr.itemattr.obj
	if attr.itemattr.parent == tree_attr.root then
	    tree:FireExtEvent("OnPopupGroupMenu", attr.itemattr)
	else
	    tree:FireExtEvent("OnPopupItemMenu", attr.itemattr)
	end
end

function UpdateVScroll(self)
	local content = self:GetControlObject("Item.Cotent")
	local left,top,right,bottom = content:GetObjPos()
	local height = bottom - top
	local vscroll = self:GetControlObject("treectrl.vscroll")
	vscroll:SetPageSize(height)

	local range = totalVisible*40 - height
	if range < 0 then
		vscroll:SetVisible(false)
		vscroll:SetChildrenVisible(false)
		range = 0
	else
		vscroll:SetVisible(true)
		vscroll:SetChildrenVisible(true)
	end
	vscroll:SetScrollRange(0, range)
end

function Item_OnLButtonDbClick(self)
	local tree = self:GetOwnerControl()
	local attr = self:GetAttribute()
	local tree_attr = tree:GetAttribute()
	local itemattr = attr.itemattr
	if attr.itemattr.parent == tree_attr.root then
	    itemattr.expand = not itemattr.expand
	else
	    tree:FireExtEvent("OnSelected", itemattr)
	end
	tree:SetAdjust()
end

function Item_OnMouseHover(self, x, y)
    local tree = self:GetOwnerControl()
	local attr = self:GetAttribute()
	local tree_attr = tree:GetAttribute()
	local itemattr = attr.itemattr
	local objx,objy = self:GetAbsPos()
	if attr.itemattr.parent ~= tree_attr.root then
	    tree:FireExtEvent("OnHover", itemattr, objy+y)
	end
end

function Item_OnMouseLeave(self)
    local tree = self:GetOwnerControl()
	local attr = self:GetAttribute()
	local tree_attr = tree:GetAttribute()
	local itemattr = attr.itemattr
	if attr.itemattr.parent ~= tree_attr.root then
	    local txt = self:GetControlObject("ItemText")
		txt:SetEffectColorResID("system.transparent")
		tree:FireExtEvent("OnLeave", itemattr.obj)
	end
end

function MouseWheel(self, distance)
	local attr = self:GetAttribute()
	local step = 1
	if distance < 0 then
		if #mapTopItem >= ItemVisibleCount then  
			attr.m_top = attr.m_top - step
		end
	else
		attr.m_top = attr.m_top + step
	end
	self:SetAdjust()
end

function SetExpand(self, item, expand,update)
	item.expand = expand
	if update then self:SetAdjust() end
end

function TreeCtrl_OnHScroll(self, fun, type, pos)
	if m_hpos then
		if m_hpos == pos then return end
	end
	m_hpos = pos
		
	local tree = self:GetOwnerControl()
	local attr = tree:GetAttribute()
	local content = tree:GetControlObject("Item.Cotent")
	local left,top,right,bottom = content:GetObjPos()
	local width = right - left
	m_left = pos / 100 * (attr.maxwidth + 30 - width) / 12
	if attr.m_left == m_left then return end
	attr.m_left = m_left
	--XLPrint(string.format("XLSpy:type=%d,pos=%d,m_left=%d,attr.maxwidth-width=%d", type,pos,attr.m_left,attr.maxwidth-width))
	tree:SetAdjust()
end

function OnPosChange(self)
	local attr = self:GetAttribute()
	local content = self:GetControlObject("Item.Cotent")
	for i=1,#attr.VisibleItems do
		content:RemoveChild(attr.VisibleItems[i])
	end
	attr.VisibleItems = {}
	local left,top,right,bottom = content:GetObjPos()
	local height = bottom - top
	ItemVisibleCount = math.ceil(height / m_itemheight)
	attr.ItemVisibleCount = ItemVisibleCount

	for i=1,ItemVisibleCount do
		local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
		local obj = objFactory:CreateUIObject("item."..i, "MainTreeItem")
		content:AddChild(obj)
		table.insert(attr.VisibleItems, obj)
	end	
	self:SetAdjust()
end

function EnsureVisible(self,item,update)
	local attr = self:GetAttribute()
	--if #attr.VisibleItems < math.abs(attr.m_top) then
		attr.m_top = attr.m_top - item.top + 1
		while item.parent do
			item = item.parent
			item.expand = true
		end
	--end
	if update then 
		self:SetAdjust() 
	end
end

function OnMouseWheel(self,x,y,distance)
	self:GetOwnerControl():MouseWheel(distance)
end

function Item_OnMouseWheel(self,x,y,distance)
	self:GetOwnerControl():MouseWheel(distance)
end

function OnScrollBarMouseWheel(self,func,x,y,distance)
	self:GetOwnerControl():MouseWheel(distance)
end

function SetItemText(self, item, txt)
	item.txt = txt
end