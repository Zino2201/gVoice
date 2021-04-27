pcall(function() require("gvoice") end)

hook.Add("Think", "GVoiceUpdate", function()
    gvoice.FlushRecognitionQueue();
end)

hook.Add("GVoice.LogImpl", "GVoiceLogImplHandler", function(msg, r, g, b, a)
    MsgC(Color(r, g, b, a), "[GVoice] ".. msg.."\n")
end)


gvoice.Undefined = 0
gvoice.Male = 1
gvoice.Female = 2
gvoice.Neutral = 3

--- Ages
gvoice.Adult = 30
gvoice.Child = 10
gvoice.Undefined = 0
gvoice.Senior = 65
gvoice.Teen = 15