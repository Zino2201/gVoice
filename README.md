gVoice
================

Simple wrapper around the .NET `System.Speech` API (**Windows Only**)

## How to install
todo

## Features
- Basic text to speech
- Basic text recognition

## How to use

### Text recognition

```lua
-- Init text recognition with the fr-FR culture (requires the fr-FR Windows language pack!)
-- Always specifiy cultures with this format (ab-CD)
gvoice.InitRecognition("fr-FR")

-- Start listening
gvoice.StartRecognition()

-- Called when speech is recognized
hook.Add("GVoice.OnSpeechRecognized", "OnSpeechRecognizedHandler", function(text)
   print(text)
end)

```

You can later one stop the recognition
```lua
gvoice.StopRecognition().
```

#### Custom recognition

You can use custom words/sentences to have a more accurate recognition if you only need to listen for a specific subset of words/sentences.
```lua
gvoice.InitRecognition("fr-FR", { "Baguette", "Bonjour", "Comment allez-vous ?" })
```

### Text to speech

```lua
gvoice.Speak("Hello !")
```
