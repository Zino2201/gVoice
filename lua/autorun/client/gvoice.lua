pcall(function() require("gvoice") end)

hook.Add("Think", "GVoiceUpdate", function()
    gvoice.FlushRecognitionQueue();
end)

hook.Add("GVoice.LogImpl", "GVoiceLogImplHandler", function(msg, r, g, b, a)
    MsgC(Color(r, g, b, a), "[GVoice] ".. msg.."\n")
end)