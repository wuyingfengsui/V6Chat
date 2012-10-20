--lua文件必须是UTF-8编码的(最好无BOM头)
function photo_OnBind(self)
    local my_photo = self:GetObject("my_photo")
	local app = XLGetObject("V6Chat3App")
	local my_head = app:GetMyHead()
	if my_head then
	    my_photo:SetBitmap(my_head)
	else
	    my_photo:SetResID("app.unknown_photo")
	end
end

function photo_OnMouseMove(self)
    self:SetCursorID ("IDC_HAND")
end

function photo_OnMouseLeave(self)
    self:SetCursorID ("IDC_ARROW")
end

function photo_OnLButtonDown(self)
    local app = XLGetObject("V6Chat3App")
	local retCode, my_head = app:SelectMyHead()
	if retCode == 0 then
	    local my_photo = self:GetObject("my_photo")
	    my_photo:SetBitmap(my_head)
		
		local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")	
		local aniAlpha = aniFactory:CreateAnimation("AlphaChangeAnimation")
		aniAlpha:BindRenderObj(my_photo)
		aniAlpha:SetTotalTime(1000)
		aniAlpha:SetKeyFrameAlpha(0,255)
		local owner = self:GetOwner()
		owner:AddAnimation(aniAlpha)
		aniAlpha:Resume()
	elseif retCode == 3 then
	    local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("图片大小超过100KB！", 250, 130)
	end
end

function close_btn_OnClick(self)
	local app = XLGetObject("V6Chat3App")
	app:Quit()
end

function nick_name_OnTextChanged(self)
	local tree = self:GetOwner()
	local sex_pr = tree:GetUIObject("sex_pr")
	if not sex_pr then
		local more_profile = tree:GetUIObject("more_profile")
		more_profile:SetVisible(true)
	end
end

function nick_name_OnBind(self)
    self:SetMaxLength(20)
	
	local app = XLGetObject("V6Chat3App")
	local name = app:GetMyInfo()
	self:SetText(name)
end


function more_profile_OnMouseMove(self)
	self:SetTextFontResID ("font.category.item.underline")
    self:SetCursorID ("IDC_HAND")
end


function more_profile_OnMouseLeave(self)
	self:SetTextFontResID ("font.category.item")
end

function create_more_profile()
	local treeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
	local tree = treeManager:GetUIObjectTree("BeginObjectTree")
	local tree_root = tree:GetUIObject("app.bkg")
	
	local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
	local xarManager = XLGetObject("Xunlei.UIEngine.XARManager")
	
	local begin_button = tree:GetUIObject("begin_button")
	local l = begin_button:GetObjPos()
	local t = 291
	
    local sex_pr = objFactory:CreateUIObject("sex_pr","TextObject")
    sex_pr:SetResProvider(xarManager)
    sex_pr:SetObjPos(l-73,t-10,l,t+15)
	sex_pr:SetText("性别：")
	tree_root:AddChild(sex_pr)
	
	local sex_man = objFactory:CreateUIObject("sex_man","Util.Radio")
    sex_man:SetResProvider(xarManager)
    sex_man:SetObjPos(l-25,t-10,l+25,t+15)
	sex_man:SetText("男")
	
	local sex_woman = objFactory:CreateUIObject("sex_woman","Util.Radio")
    sex_woman:SetResProvider(xarManager)
    sex_woman:SetObjPos(l+26,t-10,l+75,t+15)
	sex_woman:SetText("女")
	
	local function onClickSexMan()
        local treeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
		local beginTree = treeManager:GetUIObjectTree("BeginObjectTree")
		local sex_woman = beginTree:GetUIObject("sex_woman")
		sex_woman:SetState(0)
    end
    sex_man:AttachListener("OnClick",true,onClickSexMan)
	
	local function onClickSexWoman()
        local treeManager = XLGetObject("Xunlei.UIEngine.TreeManager")
		local beginTree = treeManager:GetUIObjectTree("BeginObjectTree")
		local sex_man = beginTree:GetUIObject("sex_man")
		sex_man:SetState(0)
    end
    sex_woman:AttachListener("OnClick",true,onClickSexWoman)
	
	sex_man:SetTabOrder(1)
	sex_woman:SetTabOrder(2)
	tree_root:AddChild(sex_man)
	tree_root:AddChild(sex_woman)
	
	local age = objFactory:CreateUIObject("age","Util.text_edit")
    age:SetResProvider(xarManager)
    age:SetObjPos(l-73,t+30,l,t+55)
	age:SetTitle("年龄：")
	age:SetTextWidth(40)
	age:SetTextColorResId("system.lightblue")
	age:SetIsNumber(true)
	age:SetMaxLength(3)
	age:SetTabOrder(3)
	tree_root:AddChild(age)
	
    local introduction = objFactory:CreateUIObject("introduction","Util.text_edit")
    introduction:SetResProvider(xarManager)
    introduction:SetObjPos(l-73,t+78,l+168,t+103)
	introduction:SetTitle("签名：")
	introduction:SetTextWidth(40)
	introduction:SetTextColorResId("system.lightblue")
	introduction:SetMaxLength(50)
	introduction:SetTabOrder(4)
	tree_root:AddChild(introduction)
end

function more_profile_OnLButtonDown(self)
	self:SetVisible(false)
	
	local tree = self:GetOwner()
	local tree_root = tree:GetUIObject("app.bkg")
	
	local small_map = tree:GetUIObject("small_map")
	small_map:SetVisible(false)
	
	local begin_button = tree:GetUIObject("begin_button")
	local aniFactory = XLGetObject("Xunlei.UIEngine.AnimationFactory")
    local posAni = aniFactory:CreateAnimation("PosChangeAnimation")
	posAni:SetTotalTime(200)
	posAni:BindLayoutObj(begin_button)
	local l,t = begin_button:GetObjPos()
	posAni:SetKeyFramePos(l,t,l,t+150)
	
	local function onAniFinish(self,oldState,newState)
		if newState == 4 then
			create_more_profile()
		end
	end
	
	posAni:AttachListener(true,onAniFinish)
	tree:AddAnimation(posAni)
	posAni:Resume()
end


function begin_btn_OnClick(self)
    local nick_name_edit = self:GetObject("tree:nick_name:edit")
	local nick_name = nick_name_edit:GetText()
	
	if nick_name ~= "" then
	    local owner = self:GetOwner()
	    local HostWnd = owner:GetBindHostWnd()
		HostWnd:SetVisible(false)
		
		local app = XLGetObject("V6Chat3App")
		local sex = 0
		local age = 0
		local introduction = ""
		
		local more_profile = self:GetObject("tree:more_profile")
		if not more_profile:GetVisible() then
			
			local sex_man = self:GetObject("tree:sex_man")
			local sex_woman = self:GetObject("tree:sex_woman")
			if sex_man and sex_man:GetState() == 1 then
				sex = 1
			elseif sex_woman and sex_woman:GetState() == 1 then
				sex = 2
			else
				sex = 0
			end
			
			if sex_woman then
				age = self:GetObject("tree:age:edit"):GetText()
				introduction = self:GetObject("tree:introduction:edit"):GetText()
			end
		end
		
		local ret = app:Start(nick_name, sex, age, introduction)
		
		if ret == 1 then
		    local XARManager = XLGetObject("Xunlei.UIEngine.XARManager")
		    local success = XARManager:LoadXAR("Main")
			
			if success then
	            local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
		        hostwndManager:RemoveHostWnd(HostWnd:GetID())
	            local treeMananger = XLGetObject("Xunlei.UIEngine.TreeManager")
	            treeMananger:DestroyTree(owner:GetID())
			else
			    local uiHelper = XLGetGlobal("V6ChatUIHelper")
				uiHelper:V6MessageBox("无法导入Main XAR包！", 250, 130)
				local app = XLGetObject("V6Chat3App")
				app:Quit()
			end
		else
		    HostWnd:SetVisible(true)
			local uiHelper = XLGetGlobal("V6ChatUIHelper")
			uiHelper:V6MessageBox("发生错误！代码：" ..ret, 250, 130)
		end
	else
	    local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("昵称不能为空！", 250, 130)
	end
end

function OnFocusChange(hwnd, focus)
    if focus then
	    local tree = hwnd:GetBindUIObjectTree()
		local nick_name = tree:GetUIObject("nick_name")
		local edit = nick_name:GetControlObject("edit")
	    edit:SetFocus(true)
	end
end

function OnCreate(self)
    local function MessageFilter(self, msg, wparam, lparam)
		if(msg == 0x0100 and wparam == 0x0D) then          --按下回车键
		    local tree = self:GetBindUIObjectTree()
			local begin_button = tree:GetUIObject("begin_button")
			begin_btn_OnClick(begin_button)
		elseif msg == 0x0010 then
			local app = XLGetObject("V6Chat3App")
			app:Quit()
		end
	end
	
	self:AddInputFilter(nil, MessageFilter)
	self:Center()
end

function OnDestroy(self)
end

function OnShowWindow(self, isShow)
    if isShow then
		local tree = self:GetBindUIObjectTree()
		local name = tree:GetUIObject("nick_name"):GetText()
		if name and name ~= "" then
			local more_profile = tree:GetUIObject("more_profile")
			more_profile:SetVisible(false)
		end
	end
end