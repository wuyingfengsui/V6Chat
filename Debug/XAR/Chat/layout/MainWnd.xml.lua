--lua文件必须是UTF-8编码的(最好无BOM头)

function close_btn_OnClick(self)
	local downloadingfileTable = self:GetOwner():GetBindHostWnd():GetUserData().downloadingfileTable
	if downloadingfileTable and #downloadingfileTable > 0 then
		local uiHelper = XLGetGlobal("V6ChatUIHelper")
		if uiHelper:V6MessageBox("有文件正在传输，是否关闭？", 300, 130, 2) ~= 1 then
			return
		end
	end
	
	local owner = self:GetOwner()
	local hostwndManager = XLGetObject("Xunlei.UIEngine.HostWndManager")
	local HostWnd = owner:GetBindHostWnd()
	hostwndManager:RemoveHostWnd(HostWnd:GetID())
	local treeMananger = XLGetObject("Xunlei.UIEngine.TreeManager")
	treeMananger:DestroyTree(owner:GetID())
end

function min_btn_OnClick(self)
	local hwnd = self:GetOwner():GetBindHostWnd()
	hwnd:Min()
end

function peer_head_OnBind(self)
end


function peer_head_OnClick(self)
    
end


function tree_OnBindHostWnd(tree, hostwnd, isBind)
    if isBind then
	    
	end
end

function show_file(self, fileid, filepathname, panelname)
	local owner = self:GetOwner()
	local pn = owner:GetUIObject(panelname)
	local richedit = pn:GetControlObject("edit")
	if fileid == 0 then
		local fmt = {}
		fmt.textcolor = "#ff0000"
		richedit:SetInsertCharFormat(fmt)
		richedit:AppendText("图片接收失败！\n")
		fmt.textcolor = "#130c0e"
		richedit:SetInsertCharFormat(fmt)
		return
	end
	
	local app = XLGetObject("V6Chat3App")
	local objFactory = XLGetObject("Xunlei.UIEngine.ObjectFactory")
	local xarManager = XLGetObject("Xunlei.UIEngine.XARManager")
	
	if not filepathname then    -- 文件传输失败或正在下载
		local fileItem = objFactory:CreateUIObject("file" .. "_" ..fileid, "SendFileItem")
		
		if fileItem then
			local userdata = owner:GetBindHostWnd():GetUserData()
			local downloadingfile = {}
			downloadingfile.fileid = fileid
			downloadingfile.fileItem = {}
			downloadingfile.fileItem = fileItem
			table.insert(userdata.downloadingfileTable, downloadingfile)
			
			fileItem:SetResProvider(xarManager)
			fileItem:SetObjPos(0, 0, 300, 50)
			fileItem:SetId(fileid)
			fileItem:SetState(0)
			
			richedit:InsertObject(fileItem)
			local len = richedit:GetLength()
			richedit:SetSel(len, len)
			
			owner:GetBindHostWnd():SetUserData(userdata)
			return true
		end
	else
		local nbpos, nepos = string.find(filepathname, ".*\\")
		local filepath = ""
		local filename = ""
	
		if nepos then
			filepath = string.sub(filepathname, 1, nepos)
			filename = string.sub(filepathname, nepos+1)
		end
	
		local img = app:GetImageHandle(filepathname)
		if img then       --图片文件
			local imagepng = objFactory:CreateUIObject("imagepng" .. "_" .. filename .. "_" ..fileid,"ImageObject")
			if imagepng then
				imagepng:SetResProvider(xarManager)
				local cl, w, h = img:GetInfo()
				local ctl, ctt, ctr, ctb = pn:GetObjPos()
				if ctr-ctl > w then
					imagepng:SetObjPos(0, 0, w, h)
				else
					imagepng:SetObjPos(0, 0, ctr-ctl, ((ctr-ctl)/w)*h)
					imagepng:SetDrawMode(1)
				end
				imagepng:SetBitmap(img)
			
				richedit:InsertObject(imagepng)
				local len = richedit:GetLength()
				richedit:SetSel(len, len)
			
				return true
			end
		else
			local fileItem = objFactory:CreateUIObject("file" .. "_" .. filename .. "_" ..fileid, "SendFileItem")
		
			if fileItem then
				fileItem:SetResProvider(xarManager)
				fileItem:SetObjPos(0, 0, 300, 50)
				fileItem:SetId(fileid)
				fileItem:SetFilePath(filepath)
				fileItem:SetFileName(filename)
				fileItem:SetState(1)
			
				richedit:InsertObject(fileItem)
				local len = richedit:GetLength()
				richedit:SetSel(len, len)
			
				return true
			end
		end
	end
	return false
end


function replace_file(downloadingfileTable, uid, success)        -- 更新显示的文件（文件下载完成时调用）
	local app = XLGetObject("V6Chat3App")
	for i=1,#downloadingfileTable do
		if downloadingfileTable[i] and downloadingfileTable[i].fileid then
		local pathname = app:GetRecvFile(downloadingfileTable[i].fileid, uid)
		if pathname and pathname ~= "" then
			local downloaded = downloadingfileTable[i]
			if success == 1 then
				local nbpos, nepos = string.find(pathname, ".*\\")
				local filepath = ""
				local filename = ""
	
				if nepos then
					filepath = string.sub(pathname, 1, nepos)
					filename = string.sub(pathname, nepos+1)
				end
				
				downloaded.fileItem:SetFilePath(filepath)
				downloaded.fileItem:SetFileName(filename)
			
				downloaded.fileItem:SetState(1)
			else
				downloaded.fileItem:SetState(2)
			end
			table.remove(downloadingfileTable, i)
			return downloadingfileTable
		end
		end
	end
end

function send_btn_OnClick(self)
	local owner = self:GetOwner()
	local app = XLGetObject("V6Chat3App")
	local HostWnd = owner:GetBindHostWnd()
	local sendPanel = owner:GetUIObject("sendPanel")
	local richedit = sendPanel:GetControlObject("edit")
	local text = richedit:GetRichText()
	
	if text ~= "" then
		richedit:Clear()
		
		local cp = owner:GetUIObject("chatPanel")
		richedit = cp:GetControlObject("edit")
		local len = richedit:GetLength()
		richedit:SetSel(len, len)
		
		local time = app:ChatWithText(HostWnd:GetUserData().id, text)
	
		local fmt = {}
		fmt.textcolor = "#4e72b8"
		fmt.height = 17
		richedit:SetInsertCharFormat(fmt)
		local tnpos = string.find(time, "%s")
		if tnpos then
			local htime = string.sub(time, tnpos+1)
			richedit:AppendText(app:GetMyInfo() .. "  " .. htime .. "\n")
		else
			richedit:AppendText(app:GetMyInfo() .. "  " .. time .. "\n")
		end
		fmt.textcolor = "#130c0e"
		fmt.height = 14
		richedit:SetInsertCharFormat(fmt)
	
		while text do
			local bpos, epos = string.find(text, "<%d+>")
			if bpos then
				richedit:AppendText(string.sub(text, 1, bpos-1))       --显示前半段
			
				local number = string.sub(text, bpos+1, epos-1)
				local filepathname = app:GetSendFile(number)
				if filepathname then
					show_file(self, number, filepathname, "chatPanel")  --显示文件
				else
					richedit:AppendText(string.sub(text, bpos, epos))
				end
			
				text = string.sub(text, epos+1)                      --text置为后半段
			else
				richedit:AppendText(text)
				text = nil
			end
		end
		richedit:AppendText("\n\n")
		
		richedit = sendPanel:GetControlObject("edit")
		richedit:SetFocus(true)
	end
end

function sendPanel_OnDragQuery(self, dataObject, keyState, x, y)
	XLageBox("")
	local imageCore = XLGetObject("Wizard.ImageCore")
	local ret, file = imageCore:ParseDataObject(dataObject)
	if not ret then
		return 0, true
	end
	
	return 1, true
end

local effect

function sendPanel_OnDragEnter(self, dataObject, keyState, x, y)
	effect = 0
	
	local imageCore = XLGetObject("Wizard.ImageCore")
	local ret, file = imageCore:ParseDataObject(dataObject)
	if not ret then
		return effect, true
	end
	
	effect = 1 --DROPEFFECT_COPY
	return effect, true
end

function sendPanel_OnDragOver(self)
	return effect, true
end

function sendPanel_OnDrop(self, dataObject, keyState, x, y)
	local imageCore = XLGetObject("Wizard.ImageCore")
	local ret, file = imageCore:ParseDataObject(dataObject)
	if not ret then
		return 0, true
	end
	
	local app = XLGetObject("Wizard.App")
	app:SetString("ConfigGraphics", "ConfigGraphics_BkgFile", file)
	
	return effect, true
end

function send_file_btn_OnClick(self)
	local app = XLGetObject("V6Chat3App")
	local ret, filepathname, fileId = app:SelectChatFile()
	
	if ret == 0 then
		show_file(self, fileId, filepathname, "sendPanel")
	elseif ret == 2 then
		local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("无效的文件！", 250, 130)
	elseif ret == 3 then
		local uiHelper = XLGetGlobal("V6ChatUIHelper")
		uiHelper:V6MessageBox("文件大于10MB，暂不支持大文件传输！", 400, 130)
	end
end

function show_history(self, his_tb)
	local owner = self:GetOwner()
	local hp = owner:GetUIObject("historyPanel")
	local ht = owner:GetUIObject("history_time")
	local richedit = hp:GetControlObject("edit")
	richedit:Clear()
	
	if his_tb and #his_tb > 0 then
		local npos = string.find(his_tb[1].time, "%s")
		ht:SetText(string.sub(his_tb[1].time, 1, npos-1))
		for i=1,#his_tb do
			local fmt = {}
			fmt.height = 17
			fmt.textcolor = "#007d65"
			richedit:SetInsertCharFormat(fmt)
			richedit:AppendText(his_tb[i].name)
		
			fmt.height = 10
			fmt.textcolor = "#4e72b8"
			richedit:SetInsertCharFormat(fmt)
			richedit:AppendText("  " .. his_tb[i].time .. "\n")
		
			fmt.textcolor = "#130c0e"
			fmt.height = 14
			richedit:SetInsertCharFormat(fmt)
			richedit:AppendText(his_tb[i].message .. "\n\n")
		end
		richedit:SetFocus(true)
	else
		ht:SetText("无记录")
	end
end

function history_btn_OnDownClick(self)
	local owner = self:GetOwner()
	local HostWnd = owner:GetBindHostWnd()
	
	local orl, ort, orr, orb = HostWnd:GetWindowRect()
	HostWnd:Move(orl, ort, orr - orl + 400, orb - ort)
	HostWnd:SetMinTrackSize(orr - orl + 400, orb - ort)
	
	local history_body = owner:GetUIObject("historywnd.body")
	history_body:SetVisible(true)
	history_body:SetChildrenVisible(true)
	
	local app = XLGetObject("V6Chat3App")
	local userTable = HostWnd:GetUserData()
	local his_tb, beginMid, endMid = app:GetChatHistory(userTable.id, false, 0)
	userTable.beginMid = beginMid
	userTable.endMid = endMid
	HostWnd:SetUserData(userTable)
	
	show_history(self, his_tb)
end

function history_btn_OnUpClick(self)
	local owner = self:GetOwner()
	local HostWnd = owner:GetBindHostWnd()
	local orl, ort, orr, orb = HostWnd:GetWindowRect()
	HostWnd:SetMinTrackSize(orr - orl - 400, orb - ort)
	HostWnd:Move(orl, ort, orr - orl - 400, orb - ort)
	
	local history_body = owner:GetUIObject("historywnd.body")
	history_body:SetVisible(false)
	history_body:SetChildrenVisible(false)
	
	local sp = owner:GetUIObject("sendPanel")
	local richedit = sp:GetControlObject("edit")
	richedit:SetFocus(true)
end

function history_pre_btn_OnClick(self)
	local owner = self:GetOwner()
	local HostWnd = owner:GetBindHostWnd()
	
	local app = XLGetObject("V6Chat3App")
	local userTable = HostWnd:GetUserData()
	local his_tb = {}
	local beginMid, endMid
	
	if userTable.beginMid ~= 1 then
		his_tb, beginMid, endMid = app:GetChatHistory(userTable.id, false, userTable.beginMid-1)
	end
		
	if #his_tb > 0 then
		userTable.beginMid = beginMid
		userTable.endMid = endMid
		HostWnd:SetUserData(userTable)
		
		show_history(self, his_tb)
	end
end

function history_aft_btn_OnClick(self)
	local owner = self:GetOwner()
	local HostWnd = owner:GetBindHostWnd()
	
	local app = XLGetObject("V6Chat3App")
	local userTable = HostWnd:GetUserData()
	local his_tb = {}
	local beginMid, endMid
	
	his_tb, beginMid, endMid = app:GetChatHistory(userTable.id, true, userTable.endMid+1)
	
	if #his_tb > 0 then
		userTable.beginMid = beginMid
		userTable.endMid = endMid
		HostWnd:SetUserData(userTable)
		show_history(self, his_tb)
	end
end

function history_body_OnBind(self)
	self:SetVisible(false)
	self:SetChildrenVisible(false)
end

function sendPanel_OnEditChange(self)
	local richedit = self:GetControlObject("edit")
	local text = richedit:GetText()
	if text == "\r" then
		richedit:Clear()
	end
end

function sendPanel_OnGetObjectText(self, fname, obj)
	local str = ""
	local strid = obj:GetID()
	local bpos, epos = string.find(strid, ".*[^%d]")
	if epos then
		str = string.sub(strid, epos+1)
		str = "<" .. str .. ">"
	end
	return str, true
end

function main_wnd_OnSize(self, sizetype, width, height)
	local objectTree = self:GetBindUIObjectTree()
	local history_button = objectTree:GetUIObject("history_button")
	local chatPanel = objectTree:GetUIObject("chatPanel")
	local sendPanel = objectTree:GetUIObject("sendPanel.bkg")
	local send_button = objectTree:GetUIObject("send_button")
	local rootObject = objectTree:GetRootObject()
	rootObject:SetObjPos(0, 0, width, height)
	
	if history_button:GetHasDownClick() then
		history_button:SetObjPos2(width - 490, height - 197, 75, 27)
		chatPanel:SetObjPos2(10, 100, width - 420, height - 300)
		sendPanel:SetObjPos2(10, height - 170, width - 420, 123)
		send_button:SetObjPos2(width - 490, height - 40, 75, 27)
	else
		history_button:SetObjPos2(width - 90, height - 197, 75, 27)
		chatPanel:SetObjPos2(10, 100, width - 20, height - 300)
		sendPanel:SetObjPos2(10, height - 170, width - 20, 123)
		send_button:SetObjPos2(width - 90, height - 40, 75, 27)
	end
	
	local close_button = objectTree:GetUIObject("close_button")
	local min_button = objectTree:GetUIObject("min_button")
	close_button:SetOriginalLeft(width - 50)
	min_button:SetOriginalLeft(width - 80)
end

function AppendChatContent(hostwnd)
	local tree = hostwnd:GetBindUIObjectTree()
	local cp = tree:GetUIObject("chatPanel")
	
	local app = XLGetObject("V6Chat3App")
	local txt_tb = app:GetChatContent(hostwnd:GetUserData().id)
	local richedit = cp:GetControlObject("edit")
	local len = richedit:GetLength()
	richedit:SetSel(len, len)
	
	if #txt_tb > 0 then
		local fmt = {}
		fmt.height = 17
		fmt.textcolor = "#007d65"
		richedit:SetInsertCharFormat(fmt)
		local tnpos = string.find(txt_tb[1].time, "%s")
		local name = hostwnd:GetUserData().name
		if tnpos then
			local htime = string.sub(txt_tb[1].time, tnpos+1)
			richedit:AppendText(name .. "  " .. htime .. "\n")
		else
			richedit:AppendText(name .. "  " .. txt_tb[1].time .. "\n")
		end
		fmt.textcolor = "#130c0e"
		fmt.height = 14
		richedit:SetInsertCharFormat(fmt)
	
		for i=1,#txt_tb do
			if txt_tb[i].message_type == 0 then       --系统消息
				local fmt = {}
				fmt.textcolor = "#ff0000"
				richedit:SetInsertCharFormat(fmt)
				richedit:AppendText(txt_tb[i].message)
				fmt.textcolor = "#130c0e"
				richedit:SetInsertCharFormat(fmt)
				
			elseif txt_tb[i].message_type == 1 then
				local text = txt_tb[i].message
				while text do
					local bpos, epos = string.find(text, "<%w+>")
					if bpos then
						richedit:AppendText(string.sub(text, 1, bpos-1))       --显示前半段
			
						local fid = string.sub(text, bpos+1, epos-1)
						local filepathname = app:GetRecvFile(fid, hostwnd:GetUserData().id)
						show_file(cp, fid, filepathname, "chatPanel")  --显示文件
			
						text = string.sub(text, epos+1)                      --text置为后半段
					else
						richedit:AppendText(text)
						text = nil
					end
				end
			end
			richedit:AppendText("\n")
		end
		
		richedit:AppendText("\n")
	end
end

function main_wnd_OnCreate(hostwnd)
	local function MessageFilter(self, msg, wparam, lparam)
		if msg == 0x0402 then        --更新聊天内容
			AppendChatContent(hostwnd)
			
		elseif msg == 0x0100 and wparam == 0x0D then          --按下回车键
			local tree = self:GetBindUIObjectTree()
			local send_button = tree:GetUIObject("send_button")
			send_btn_OnClick(send_button)
			
		elseif msg == 0x0401 and wparam == 3 then             --更新好友信息
			local app = XLGetObject("V6Chat3App")
			local userId = self:GetUserData().id
			local userData = app:GetUserInfoByID(userId)
			if userData then
				userData.id = userId
				self:SetUserData(userData)
				
				local tree = self:GetBindUIObjectTree()
				local peer_head = tree:GetUIObject("peer_head")
				peer_head:SetHeadImage(userData.head)
			
				local peer_name = tree:GetUIObject("peer_name")
				self:SetTitle(userData.name)
				peer_name:SetText(userData.name)
			
				local introduction = tree:GetUIObject("introduction")
				introduction:SetText(userData.introduction)
			end
		elseif msg == 0x0403 then
			local userdata = hostwnd:GetUserData()
			userdata.downloadingfileTable = replace_file(userdata.downloadingfileTable, self:GetUserData().id, lparam)
			hostwnd:SetUserData(userdata)
		end
	end
	
	local cookie, ret = hostwnd:AddInputFilter(nil, MessageFilter)
	
	hostwnd:Center()
end

function main_wnd_OnDestroy(hostwnd)
end

function main_wnd_OnShowWindow(self, isShow)
    if isShow then
		local app = XLGetObject("V6Chat3App")
		local userData = app:GetUserInfoByID(self:GetUserData())
		local tree = self:GetBindUIObjectTree()
		
		if userData then
			userData.id = self:GetUserData()
			userData.downloadingfileTable = {}           -- 正在下载的文件信息集合
			self:SetUserData(userData)
			
			local peer_head = tree:GetUIObject("peer_head")
			local cl
			local w = 0
			local h = 0
			if userData.head then
				cl, w, h = userData.head:GetInfo()
			end
			
			if h ~= 0 and w ~= 0 then
				if w > 80 then
					h = 80*h/w
					w = 80
					peer_head:SetImagePos(w, h)
				end
				
				if h > 95 then
					w = 95*w/h
					h = 95
					peer_head:SetImagePos(w, h)
				end
			end
			
			peer_head:SetHeadImage(userData.head)
			
			self:SetTitle(userData.name)
			
			local peer_name = tree:GetUIObject("peer_name")
			peer_name:SetText(userData.name)
			
			local introduction = tree:GetUIObject("introduction")
			introduction:SetText(userData.introduction)
			
			AppendChatContent(self)
		end
		
		local cp = tree:GetUIObject("chatPanel")
		local hp = tree:GetUIObject("historyPanel")
			
		cp:SetReadOnly(true)
		hp:SetReadOnly(true)
		local fmt = {facename = "幼圆"}
		local richedit = cp:GetControlObject("edit")
		richedit:SetInsertCharFormat(fmt)
		richedit = hp:GetControlObject("edit")
		richedit:SetInsertCharFormat(fmt)
	end
end

function main_wnd_OnFocusChange(hwnd, focus)
    if focus then
	    local tree = hwnd:GetBindUIObjectTree()
		local sendPanel = tree:GetUIObject("sendPanel")
	    sendPanel:SetFocus(true)
	end
end