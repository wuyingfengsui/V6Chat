function Menu_Item_OnInitControl(self)
	local attr = self:GetAttribute()
	self:SetText(attr.Text)
	if attr.SubMenuTemplate then
		local tosubObj = self:GetControlObject("tosub")
		tosubObj:SetVisible(true)
	end
end

function Menu_Item_OnLButtonDown(self)
	local attr = self:GetAttribute() 
	if not attr.SubMenuTemplate then
		self:FireExtEvent("OnSelected")
		self:GetOwner():GetBindHostWnd():EndMenu()
	end
end

function Menu_Item_SetEntered(self,entered)
	local hoverBkg = self:GetControlObject("hoverBkg")
	hoverBkg:SetVisible(entered)
end

function Menu_Item_OnMouseMove(self)
    local hoverBkg = self:GetControlObject("hoverBkg")
	hoverBkg:SetVisible(true)
end

function Menu_Item_OnMouseLeave(self)
	local attr = self:GetAttribute()
	attr.mouseHovering = false
	self:SetEntered(false)
	
	local hoverBkg = self:GetControlObject("hoverBkg")
	hoverBkg:SetVisible(false)
end


function Menu_Item_OnMouseEnter(self)
	local attr = self:GetAttribute() 
	attr.itemContainer:ChangeEnteredItem(self)
	attr.mouseHovering = true
	SetOnceTimer(function()
		if attr.mouseHovering then
			attr.itemContainer:ChangeHoveringItem(self)
		end
	end, 500)
end

function Menu_Item_SetContainer(self,itemContainer)
	local attr = self:GetAttribute()
	attr.itemContainer = itemContainer
end

function Menu_Item_PopupSubMenu(self)
	local attr = self:GetAttribute()
	if attr and attr.SubMenuTemplate then
		local templateMananger = XLGetObject("Xunlei.UIEngine.TemplateManager")		
		local subMenuTemplate = templateMananger:GetTemplate(attr.SubMenuTemplate,"ObjectTemplate")
		local controlId = self:GetID()
		if not attr.subMenuObj then
			local subMenuObj = subMenuTemplate:CreateInstance(controlId.."."..attr.SubMenuTemplate)
			if subMenuObj then
				self:AddChild(subMenuObj)
				subMenuObj:SetZorder(500)
				local left, top ,right , bottom = subMenuObj:GetObjPos()
				subMenuObj:SetObjPos("father.width-5", "5", right - left, bottom - top)
				attr.subMenuObj = subMenuObj
			end
		end
		return true
	end
	return false
end

function Menu_Item_DestroySubMenu(self)
	local attr = self:GetAttribute()
	if attr.SubMenuTemplate then
		local controlId = self:GetID()
		if attr.subMenuObj then
			self:RemoveChild(attr.subMenuObj)
			attr.subMenuObj  = nil
		end
		return true
	end
	return false
end

function Menu_Item_SetText(self,text)
	local attr = self:GetAttribute()
	attr.Text = text
	local textObj = self:GetControlObject("text")
	textObj:SetText(attr.Text)
end

function Menu_Item_GetText(self)
	local attr = self:GetAttribute()
	return attr.Text
end