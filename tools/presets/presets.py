# Refonte des presets Orage (v18) : table de donnees -> code C++
# ftype : 0 MS-20, 1 SEM, 2 Ladder, 3 MS-10 circuit, 4 Ladder CEM3320 circuit, 5 SEM circuit
# vagues : 0 saw, 1 pulse, 2 triangle ; octave : 1 = -1, 2 = 0, 3 = +1 ; assign : 0 poly 1 mono 2 unison 3 share 4 chord
P = []
def preset(name, cat, d, mods=(), steps=None, seq=None, lanes=(), test=None):
    P.append(dict(name=name, cat=cat, d=d, mods=list(mods), steps=steps, seq=seq, lanes=list(lanes), test=test))

# ----------------------------------------------------------------------- PADS
preset("Velvet Dawn", "Pad", dict(s_wave1=0, s_wave2=0, s_tune2=0.12, s_lvl1=0.6, s_lvl2=0.55, s_ftype=5, s_lpf=0.58, s_lpfRes=0.22, s_fltEnv=0.1, s_drive=0.05,
      s_aA=0.72, s_aD=0.6, s_aS=0.9, s_aR=0.85, s_l1Rate=0.12, power=0.1, bbdTime=0.12, bbdMod=1, bbdMix=0.3, bbdFb=0.1, bbdPing=0, revMix=0.4, revDecay=0.78, revSize=0.7, revDamp=0.45),
       [("MsLfo1", "MdLpf", 0.1), ("MsPolyAT", "MdLpf", 0.25)])
preset("Glass Cathedral", "Pad", dict(s_wave1=0, s_wave2=1, s_oct2=3, s_lvl1=0.6, s_lvl2=0.35, s_tune2=0.1, s_ftype=4, s_lpf=0.52, s_lpfRes=0.3, s_fltEnv=0.12,
      s_aA=0.7, s_aS=0.9, s_aR=0.9, revMix=0.55, revDecay=0.92, revSize=0.9, revPre=0.25, revDamp=0.5, revShimmer=0.3, revMod=0.4),
       [("MsWheel", "MdRevMix", 0.3), ("MsPolyAT", "MdLpf", 0.3)])
preset("Ocean Floor", "Pad", dict(s_wave1=0, s_oct1=1, s_wave2=1, s_lvl2=0.45, s_tune2=0.1, s_ftype=4, s_lpf=0.3, s_lpfRes=0.4, s_fltEnv=0.4, s_fA=0.8, s_fD=0.8, s_fS=0.5,
      s_aA=0.75, s_aS=0.9, s_aR=0.88, s_l2Rate=0.1, power=0.2, revMix=0.45, revDecay=0.88, revDamp=0.7, revSize=0.85),
       [("MsLfo2", "MdPw2", 0.35), ("MsPolyAT", "MdLpf", 0.3)])
preset("Tidal Choir", "Pad", dict(s_wave1=0, s_wave2=0, s_tune2=0.08, s_lvl1=0.55, s_lvl2=0.5, s_noise=0.06, s_ftype=5, s_semMorph=0.3, s_lpf=0.5, s_lpfRes=0.55,
      s_aA=0.7, s_aS=0.9, s_aR=0.85, s_l1Rate=0.14, s_l2Rate=0.09, revMix=0.5, revDecay=0.85, revSize=0.85, revShimmer=0.2),
       [("MsLfo1", "MdMorph", 0.25), ("MsLfo2", "MdLpf", 0.1)])
preset("Amber Strings", "Pad", dict(s_wave1=0, s_wave2=0, s_tune2=0.2, s_lvl1=0.6, s_lvl2=0.6, s_noise=0.02, s_ftype=5, s_lpf=0.62, s_lpfRes=0.18,
      s_aA=0.6, s_aS=0.9, s_aR=0.75, bbdTime=0.12, bbdFb=0.1, bbdMod=1, bbdMix=0.35, bbdPing=0, revMix=0.3, revDecay=0.65, compType=1, compAmt=0.25, power=0.15, s_l2Rate=0.3),
       [("MsVel", "MdLpf", 0.1), ("MsLfo2", "MdPitch", 0.008)])
preset("Cold Aurora", "Pad", dict(s_wave1=2, s_wave2=1, s_oct2=3, s_lvl1=0.7, s_lvl2=0.3, s_tune2=0.07, s_ftype=3, s_lpf=0.6, s_lpfRes=0.3, s_fltEnv=0.08,
      s_aA=0.8, s_aS=1.0, s_aR=0.9, s_l3Rate=0.1, revMix=0.55, revDecay=0.95, revSize=1.0, revShimmer=0.55, revDamp=0.35, revPre=0.15),
       [("MsLfo3", "MdPan", 0.4), ("MsPolyAT", "MdRevMix", 0.25)])
preset("Dust Bowl", "Pad", dict(s_wave1=0, s_wave2=0, s_oct2=1, s_tune2=0.15, s_lvl2=0.6, s_noise=0.12, s_ftype=4, s_lpf=0.4, s_lpfRes=0.3, s_drive=0.35,
      s_aA=0.55, s_aS=0.9, s_aR=0.7, power=0.3, bbdTime=0.4, bbdFb=0.3, bbdMix=0.25, bbdTone=0.4, bbdMod=0.5, revMix=0.35, revDecay=0.7, compType=1, compAmt=0.3),
       [("MsLfo1", "MdLpf", 0.06)])
preset("Moon Sync", "Pad", dict(s_wave1=0, s_wave2=0, s_sync=1, s_tune2=5, s_lvl1=0.5, s_lvl2=0.5, s_ftype=5, s_lpf=0.6, s_lpfRes=0.3,
      s_aA=0.65, s_aS=0.9, s_aR=0.85, s_l1Rate=0.1, revMix=0.45, revDecay=0.78, revShimmer=0.25),
       [("MsLfo1", "MdPitch2", 0.35), ("MsPolyAT", "MdLpf", 0.3)])
preset("Warm Brass Pad", "Pad", dict(s_wave1=0, s_wave2=0, s_tune2=0.1, s_lvl1=0.65, s_lvl2=0.55, s_ftype=4, s_lpf=0.38, s_lpfRes=0.25, s_fltEnv=0.45, s_fA=0.35, s_fD=0.5, s_fS=0.6,
      s_aA=0.4, s_aS=0.9, s_aR=0.55, power=0.2, compType=1, compAmt=0.3, revMix=0.25, revDecay=0.6),
       [("MsVel", "MdFltEnv", 0.2)])
preset("Hollow Wind", "Pad", dict(s_wave1=2, s_lvl1=0.4, s_noise=0.3, s_ftype=5, s_semMorph=0.45, s_lpf=0.55, s_lpfRes=0.6,
      s_aA=0.8, s_aS=0.95, s_aR=0.9, s_l1Rate=0.15, s_l2Rate=0.07, revMix=0.5, revDecay=0.88, revSize=0.9),
       [("MsLfo1", "MdMorph", 0.2), ("MsLfo2", "MdLpf", 0.15), ("MsPolyAT", "MdNoise", 0.3)])
preset("Slow Motion Chord", "Pad", dict(s_wave1=2, s_wave2=0, s_tune2=0.12, s_lvl2=0.5, s_ftype=5, s_lpf=0.5, s_lpfRes=0.2, s_assign=4, s_uniDetune=0.5,
      s_aA=0.55, s_aS=0.9, s_aR=0.75, s_l1Rate=0.1, revMix=0.35, revDecay=0.72, power=0.15),
       [("MsLfo1", "MdLpf", 0.1)])
preset("Giant Space", "Pad", dict(s_wave1=0, s_oct1=1, s_wave2=0, s_oct2=3, s_lvl2=0.4, s_tune2=0.09, s_ftype=4, s_lpf=0.45, s_lpfRes=0.3,
      s_aA=0.85, s_aS=1.0, s_aR=0.95, revMix=0.65, revDecay=0.96, revSize=1.0, revShimmer=0.4, revDamp=0.4, revPre=0.2, power=0.1),
       [("MsLfo1", "MdPan", 0.3)])
preset("Pulse Silk", "Pad", dict(s_wave1=1, s_wave2=1, s_tune2=0.15, s_lvl1=0.5, s_lvl2=0.5, s_ftype=5, s_lpf=0.62, s_lpfRes=0.25, s_l2Rate=0.22, s_l2Wave=1,
      s_aA=0.5, s_aS=0.9, s_aR=0.7, bbdTime=0.12, bbdMod=1, bbdMix=0.3, bbdFb=0.1, bbdPing=0, revMix=0.35, revDecay=0.7),
       [("MsLfo2", "MdPw1", 0.4), ("MsLfo2", "MdPw2", -0.4)])
preset("Blue Hour", "Pad", dict(s_wave1=0, s_wave2=0, s_tune2=0.14, s_lvl2=0.6, s_ftype=3, s_lpf=0.5, s_lpfRes=0.25, s_fltEnv=0.15, s_fA=0.6,
      s_aA=0.6, s_aS=0.9, s_aR=0.8, bbdMix=0.3, bbdSync=6, bbdFb=0.35, bbdPing=1, revMix=0.45, revDecay=0.8, power=0.1),
       [("MsLfo1", "MdLpf", 0.08)])

# ----------------------------------------------------------------------- BASS
preset("Ladder Sub", "Bass", dict(s_wave1=2, s_oct1=1, s_wave2=1, s_oct2=1, s_lvl2=0.5, s_ftype=4, s_lpf=0.33, s_lpfRes=0.3, s_fltEnv=0.4, s_fD=0.3, s_fS=0.1,
      s_aS=0.9, s_aR=0.15, power=0.4, compType=1, compAmt=0.35), [])
preset("Pro Pluck Bass", "Bass", dict(s_wave1=0, s_oct1=1, s_ftype=4, s_lpf=0.3, s_lpfRes=0.45, s_fltEnv=0.75, s_fD=0.18, s_fS=0.0, s_aD=0.3, s_aS=0.5, s_aR=0.12,
      power=0.35, compType=1, compAmt=0.4), [("MsVel", "MdFltEnv", 0.25)])
preset("Acid MS-10", "Bass", dict(s_wave1=0, s_oct1=1, s_lvl1=0.8, s_ftype=3, s_lpf=0.28, s_lpfRes=0.85, s_fltEnv=0.6, s_fD=0.22, s_fS=0.0, s_drive=0.5, s_glide=0.15, s_assign=1,
      s_aS=0.7, s_aR=0.12, power=0.35, compType=1, compAmt=0.45), [("MsVel", "MdFltEnv", 0.3)])
preset("Reese Deep", "Bass", dict(s_wave1=0, s_oct1=1, s_wave2=0, s_oct2=1, s_tune2=0.18, s_lvl2=0.85, s_ftype=5, s_lpf=0.4, s_lpfRes=0.2, s_l1Rate=0.2,
      s_aS=0.9, s_aR=0.15, power=0.5, compType=1, compAmt=0.4, s_drive=0.3), [("MsLfo1", "MdLpf", 0.12)])
preset("Growl FM", "Bass", dict(s_wave1=2, s_oct1=1, s_wave2=2, s_oct2=1, s_fm=0.5, s_ring=0.15, s_lvl2=0.5, s_ftype=3, s_lpf=0.55, s_lpfRes=0.6, s_drive=0.5,
      s_l1Rate=0.5, s_l1Sync=6, s_aS=0.9, s_aR=0.15, power=0.55, clip=0.3, compType=1, compAmt=0.35),
       [("MsLfo1", "MdFm", 0.35), ("MsLfo1", "MdLpf", 0.15)])
preset("Fat Moog", "Bass", dict(s_wave1=0, s_oct1=1, s_wave2=1, s_oct2=1, s_lvl2=0.5, s_pw2=0.45, s_ftype=4, s_lpf=0.38, s_lpfRes=0.55, s_fltEnv=0.55, s_fD=0.25, s_fS=0.15, s_drive=0.35,
      s_glide=0.1, s_assign=1, s_aS=0.85, s_aR=0.15, power=0.3, compType=1, compAmt=0.4), [("MsVel", "MdFltEnv", 0.2)])
preset("Sync Snarl", "Bass", dict(s_wave1=0, s_oct1=1, s_wave2=0, s_oct2=1, s_sync=1, s_tune2=8, s_lvl2=0.6, s_ftype=3, s_lpf=0.5, s_lpfRes=0.4, s_fltEnv=0.35, s_fD=0.25, s_fS=0.2,
      s_assign=1, s_aS=0.9, s_aR=0.15, power=0.5, compType=1, compAmt=0.35), [("MsEnv", "MdPitch2", 0.5)])
preset("Wobble Dub", "Bass", dict(s_wave1=0, s_oct1=1, s_wave2=0, s_oct2=1, s_tune2=0.1, s_lvl2=0.5, s_ftype=4, s_lpf=0.32, s_lpfRes=0.6, s_l1Rate=0.5, s_l1Sync=7, s_l1Wave=1,
      s_assign=1, s_aS=0.9, s_aR=0.15, power=0.4, compType=1, compAmt=0.4), [("MsLfo1", "MdLpf", 0.16)])
preset("Sub Drop", "Bass", dict(s_wave1=2, s_oct1=0, s_wave2=1, s_oct2=1, s_lvl2=0.3, s_ftype=4, s_lpf=0.3, s_lpfRes=0.15, s_fltEnv=0.1,
      s_aD=0.7, s_aS=0.6, s_aR=0.2, s_fD=0.3, s_fS=0.2, power=0.2, compType=1, compAmt=0.3), [("MsAmpEnv", "MdPitch", 0.09)])
preset("Hoover Low", "Bass", dict(s_wave1=0, s_oct1=1, s_wave2=0, s_oct2=1, s_tune2=0.3, s_lvl2=0.8, s_ftype=5, s_semMorph=0.05, s_lpf=0.52, s_lpfRes=0.35,
      s_assign=2, s_uniDetune=0.6, s_glide=0.08, s_aS=0.9, s_aR=0.2, power=0.45, compType=1, compAmt=0.35), [("MsEnv", "MdPitch", 0.06)])
preset("Fold Bass", "Bass", dict(s_wave1=2, s_oct1=1, s_wave2=2, s_oct2=1, s_lvl2=0.5, s_fold=0.45, s_foldSym=0.2, s_ftype=4, s_lpf=0.5, s_lpfRes=0.3, s_fltEnv=0.3, s_fD=0.3, s_fS=0.2,
      s_aS=0.9, s_aR=0.15, power=0.3, compType=1, compAmt=0.35), [("MsEnv", "MdFold", 0.4)])
preset("Square Bass 78", "Bass", dict(s_wave1=1, s_oct1=1, s_pw1=0.5, s_lvl1=0.8, s_ftype=3, s_lpf=0.38, s_lpfRes=0.35, s_fltEnv=0.5, s_fD=0.22, s_fS=0.1,
      s_aD=0.35, s_aS=0.6, s_aR=0.1, power=0.25, compType=1, compAmt=0.4), [])
preset("Dirty Rubber", "Bass", dict(s_wave1=0, s_oct1=1, s_wave2=0, s_oct2=1, s_tune2=0.1, s_lvl2=0.5, s_ftype=5, s_lpf=0.3, s_lpfRes=0.7, s_fltEnv=0.7, s_fD=0.25, s_fS=0.05, s_drive=0.6,
      s_aS=0.8, s_aR=0.12, power=0.5, clip=0.3, compType=1, compAmt=0.45), [("MsVel", "MdFltEnv", 0.2)])
preset("Ladder Squelch", "Bass", dict(s_wave1=0, s_oct1=1, s_lvl1=0.85, s_ftype=4, s_lpf=0.25, s_lpfRes=0.82, s_fltEnv=0.65, s_fD=0.2, s_fS=0.0, s_drive=0.3, s_glide=0.12, s_assign=1,
      s_aS=0.7, s_aR=0.1, power=0.3, compType=1, compAmt=0.4), [("MsVel", "MdFltEnv", 0.3)])
preset("Seq Bass Line", "Bass", dict(s_wave1=0, s_oct1=1, s_wave2=0, s_oct2=1, s_tune2=0.06, s_lvl2=0.5, s_ftype=3, s_lpf=0.32, s_lpfRes=0.6, s_fltEnv=0.45, s_fD=0.18, s_fS=0.05,
      s_aD=0.3, s_aS=0.3, s_aR=0.08, s_drive=0.4, s_arpOn=1, s_assign=1, s_arpRate=0.7, s_arpGate=0.5, s_arpSwing=0.2, power=0.3, compType=1, compAmt=0.4),
       steps="2112112121121121", seq=([36, 36, 48, 36, 39, 36, 51, 36, 34, 34, 46, 34, 41, 36, 48, 43], "0000100000001000"),
       lanes=[(0, "MdLpf", 0.5, 0.15, [0.2, 0.4, 0.9, 0.3, 0.5, 0.2, 1.0, 0.4])])

# ----------------------------------------------------------------------- LEADS
preset("Solo SEM", "Lead", dict(s_wave1=0, s_wave2=0, s_tune2=0.1, s_lvl2=0.6, s_ftype=5, s_lpf=0.65, s_lpfRes=0.6, s_glide=0.12, s_l1Rate=0.55, s_l1Fade=0.5, s_l1Trig=1,
      s_aA=0.3, s_aS=0.9, s_aR=0.45, power=0.2, bbdMix=0.25, bbdTime=0.5, bbdFb=0.35, revMix=0.2, revDecay=0.5),
       [("MsLfo1", "MdPitch", 0.02), ("MsPolyAT", "MdLpf", 0.3)])
preset("Screaming MS-10", "Lead", dict(s_wave1=0, s_wave2=0, s_tune2=0.08, s_lvl2=0.5, s_ftype=3, s_lpf=0.6, s_lpfRes=0.7, s_drive=0.45, s_glide=0.15, s_fltEnv=0.2, s_fD=0.4, s_fS=0.4,
      s_aS=0.9, s_aR=0.3, power=0.35, bbdMix=0.25, bbdTime=0.5, bbdFb=0.35, revMix=0.2),
       [("MsPolyAT", "MdLpf", 0.2), ("MsWheel", "MdReso", 0.15)])
preset("Ladder Sync Lead", "Lead", dict(s_wave1=0, s_wave2=0, s_sync=1, s_tune2=7, s_lvl2=0.5, s_ftype=4, s_lpf=0.7, s_lpfRes=0.55, s_glide=0.18, s_drive=0.4,
      s_aS=0.9, s_aR=0.3, power=0.4, revMix=0.15), [("MsEnv", "MdPitch2", 0.4)])
preset("Soft Sine Lead", "Lead", dict(s_wave1=2, s_lvl1=0.85, s_ftype=5, s_lpf=0.85, s_lpfRes=0.1, s_glide=0.1, s_l1Rate=0.5, s_l1Fade=0.6, s_l1Trig=1,
      s_aA=0.25, s_aS=0.95, s_aR=0.4, power=0.1, revMix=0.3, revDecay=0.6, revShimmer=0.1), [("MsLfo1", "MdPitch", 0.015)])
preset("Pulse Width Lead", "Lead", dict(s_wave1=1, s_wave2=1, s_tune2=0.15, s_lvl2=0.5, s_ftype=5, s_lpf=0.62, s_lpfRes=0.4, s_l2Rate=0.25, s_l2Wave=1,
      s_aS=0.9, s_aR=0.3, power=0.3, bbdMix=0.3, bbdSync=6, bbdFb=0.4),
       [("MsLfo2", "MdPw1", 0.5), ("MsLfo2", "MdPw2", -0.5)])
preset("Wavefold Lead", "Lead", dict(s_wave1=2, s_wave2=0, s_lvl2=0.35, s_tune2=0.1, s_fold=0.5, s_foldSym=0.2, s_ftype=3, s_lpf=0.75, s_lpfRes=0.4, s_aS=0.9, s_aR=0.3,
      s_l1Rate=0.3, bbdMix=0.3, bbdTime=0.55, bbdFb=0.45, revMix=0.15),
       [("MsEnv", "MdFold", 0.5), ("MsLfo1", "MdFold", 0.25)])
preset("Unison Monster", "Lead", dict(s_wave1=0, s_wave2=0, s_tune2=0.1, s_lvl2=0.7, s_ftype=4, s_lpf=0.55, s_lpfRes=0.3, s_fltEnv=0.3, s_fD=0.3, s_fS=0.4,
      s_assign=2, s_uniDetune=0.4, s_glide=0.08, s_aS=0.9, s_aR=0.25, power=0.3, revMix=0.15), [("MsPolyAT", "MdLpf", 0.2)])
preset("FM Brass Lead", "Lead", dict(s_wave1=0, s_wave2=2, s_fm=0.3, s_lvl2=0.4, s_tune2=0.05, s_ftype=4, s_lpf=0.6, s_lpfRes=0.2, s_fltEnv=0.35, s_fA=0.2, s_fD=0.4, s_fS=0.5,
      s_aA=0.15, s_aS=0.9, s_aR=0.3, power=0.25, revMix=0.2), [("MsEnv", "MdFm", 0.25), ("MsVel", "MdFm", 0.15)])
preset("Whistle", "Lead", dict(s_wave1=2, s_lvl1=0.85, s_ftype=5, s_semMorph=0.5, s_lpf=0.72, s_lpfRes=0.72, s_glide=0.1, s_l1Rate=0.5, s_l1Fade=0.5, s_l1Trig=1,
      s_aA=0.2, s_aS=0.95, s_aR=0.4, revMix=0.3, revDecay=0.65), [("MsLfo1", "MdPitch", 0.012), ("MsPolyAT", "MdMorph", 0.25)])
preset("Glide Mono", "Lead", dict(s_wave1=0, s_wave2=1, s_lvl2=0.4, s_tune2=0.1, s_ftype=3, s_lpf=0.6, s_lpfRes=0.45, s_glide=0.3, s_assign=1, s_fltEnv=0.3, s_fD=0.4, s_fS=0.4,
      s_aS=0.9, s_aR=0.3, power=0.3, bbdMix=0.2, bbdTime=0.45, bbdFb=0.3, revMix=0.15), [("MsVel", "MdLpf", 0.1)])
preset("Aftertouch Wail", "Lead", dict(s_wave1=0, s_wave2=0, s_tune2=0.1, s_lvl2=0.5, s_ftype=3, s_lpf=0.5, s_lpfRes=0.7, s_drive=0.4, s_glide=0.1,
      s_aS=0.9, s_aR=0.35, power=0.35, revMix=0.25), [("MsPolyAT", "MdLpf", 0.3), ("MsPolyAT", "MdReso", -0.1)])
preset("Hard Sync Sweep", "Lead", dict(s_wave1=0, s_wave2=0, s_sync=1, s_tune2=5, s_lvl1=0.6, s_lvl2=0.5, s_ftype=3, s_lpf=0.65, s_lpfRes=0.4, s_l1Rate=0.4, s_l1Wave=2,
      s_aS=0.9, s_aR=0.3, power=0.4, bbdMix=0.2, bbdTime=0.5, revMix=0.15), [("MsLfo1", "MdPitch2", 0.4)])
preset("Cry Baby", "Lead", dict(s_wave1=0, s_lvl1=0.8, s_ftype=5, s_semMorph=0.0, s_lpf=0.4, s_lpfRes=0.7, s_fltEnv=0.5, s_fA=0.1, s_fD=0.5, s_fS=0.3,
      s_aS=0.9, s_aR=0.3, power=0.2, revMix=0.2), [("MsWheel", "MdLpf", 0.3), ("MsVel", "MdFltEnv", 0.2)])
preset("Ring Lead", "Lead", dict(s_wave1=0, s_wave2=2, s_ring=0.5, s_tune2=7, s_lvl1=0.6, s_lvl2=0.4, s_ftype=5, s_lpf=0.7, s_lpfRes=0.35,
      s_aS=0.9, s_aR=0.3, power=0.25, bbdMix=0.2, bbdTime=0.5, revMix=0.2), [("MsLfo1", "MdPitch", 0.01)])

# ----------------------------------------------------------------------- PLUCK / KEYS
preset("Glass Pluck", "Pluck / Keys", dict(s_wave1=2, s_wave2=2, s_oct2=3, s_lvl2=0.4, s_ftype=3, s_lpf=0.5, s_lpfRes=0.3, s_fltEnv=0.7, s_fD=0.15, s_fS=0.0,
      s_aD=0.35, s_aS=0.0, s_aR=0.3, revMix=0.3, revDecay=0.65, revShimmer=0.2), [("MsVel", "MdFltEnv", 0.2)])
preset("Ladder Keys", "Pluck / Keys", dict(s_wave1=0, s_wave2=1, s_lvl2=0.4, s_tune2=0.1, s_ftype=4, s_lpf=0.4, s_lpfRes=0.35, s_fltEnv=0.5, s_fD=0.3, s_fS=0.3,
      s_aD=0.45, s_aS=0.3, s_aR=0.25, power=0.2, revMix=0.2, compType=1, compAmt=0.25), [("MsVel", "MdFltEnv", 0.3)])
preset("Marimba Ring", "Pluck / Keys", dict(s_wave1=2, s_wave2=2, s_ring=0.7, s_tune2=12, s_lvl1=0.7, s_lvl2=0.3, s_ftype=5, s_lpf=0.6, s_lpfRes=0.1,
      s_aD=0.28, s_aS=0.0, s_aR=0.2, revMix=0.25, revDecay=0.5), [])
preset("FM E-Piano", "Pluck / Keys", dict(s_wave1=2, s_wave2=2, s_fm=0.35, s_fmRatio=3, s_lvl1=0.7, s_fmDec=0.4, s_fmEnvAmt=0.5, s_ftype=5, s_lpf=0.75, s_lpfRes=0.1,
      s_aD=0.5, s_aS=0.3, s_aR=0.3, revMix=0.2, revDecay=0.55, bbdMix=0.2, bbdTime=0.12, bbdMod=1, bbdFb=0.1, bbdPing=0), [("MsVel", "MdFm", 0.2)])
preset("FM Bell", "Pluck / Keys", dict(s_wave1=2, s_wave2=2, s_fm=0.5, s_fmRatio=11, s_lvl1=0.7, s_fmDec=0.55, s_fmEnvAmt=0.6, s_ftype=5, s_lpf=0.85, s_lpfRes=0.05,
      s_aD=0.65, s_aS=0.0, s_aR=0.5, revMix=0.4, revDecay=0.8, revShimmer=0.25), [])
preset("Electric Clav", "Pluck / Keys", dict(s_wave1=1, s_pw1=0.35, s_lvl1=0.8, s_ftype=3, s_lpf=0.4, s_lpfRes=0.5, s_fltEnv=0.8, s_fD=0.12, s_fS=0.0,
      s_aD=0.22, s_aS=0.0, s_aR=0.12, power=0.2), [("MsVel", "MdFltEnv", 0.3)])
preset("Harpsichord", "Pluck / Keys", dict(s_wave1=0, s_wave2=1, s_oct2=3, s_lvl2=0.3, s_ftype=5, s_lpf=0.68, s_lpfRes=0.2, s_fltEnv=0.3, s_fD=0.1, s_fS=0.1,
      s_aD=0.25, s_aS=0.0, s_aR=0.15, revMix=0.2, revDecay=0.45), [])
preset("Wurly Drive", "Pluck / Keys", dict(s_wave1=2, s_wave2=0, s_lvl2=0.25, s_ftype=5, s_lpf=0.62, s_lpfRes=0.15, s_drive=0.45, s_l2Rate=0.5,
      s_aD=0.55, s_aS=0.45, s_aR=0.25, power=0.25, bbdMix=0.2, bbdTime=0.1, bbdMod=1, bbdFb=0.1, bbdPing=0, revMix=0.15), [("MsLfo2", "MdLevel", 0.12), ("MsVel", "MdDrive", 0.25)])
preset("Kalimba", "Pluck / Keys", dict(s_wave1=2, s_oct1=3, s_lvl1=0.8, s_wave2=2, s_oct2=2, s_lvl2=0.2, s_ftype=5, s_lpf=0.7, s_lpfRes=0.05,
      s_aD=0.3, s_aS=0.0, s_aR=0.35, revMix=0.35, revDecay=0.6), [])
preset("Brass Stab", "Pluck / Keys", dict(s_wave1=0, s_wave2=0, s_tune2=0.12, s_lvl2=0.6, s_ftype=4, s_lpf=0.38, s_lpfRes=0.25, s_fltEnv=0.6, s_fA=0.15, s_fD=0.25, s_fS=0.5,
      s_aA=0.1, s_aD=0.4, s_aS=0.6, s_aR=0.2, power=0.2, compType=1, compAmt=0.3, revMix=0.2), [("MsVel", "MdFltEnv", 0.25)])
preset("Vibes", "Pluck / Keys", dict(s_wave1=2, s_wave2=2, s_oct2=3, s_lvl2=0.3, s_ftype=5, s_lpf=0.7, s_lpfRes=0.1, s_l2Rate=0.45,
      s_aD=0.55, s_aS=0.0, s_aR=0.5, revMix=0.3, revDecay=0.65), [("MsLfo2", "MdLevel", 0.2)])
preset("Pluck Pad Hybrid", "Pluck / Keys", dict(s_wave1=0, s_wave2=1, s_lvl2=0.5, s_tune2=0.1, s_ftype=4, s_lpf=0.35, s_lpfRes=0.4, s_fltEnv=0.55, s_fD=0.4, s_fS=0.2,
      s_aA=0.1, s_aD=0.6, s_aS=0.4, s_aR=0.5, revMix=0.35, revDecay=0.75, bbdMix=0.25, bbdSync=6, bbdFb=0.35, bbdPing=1), [])

# ----------------------------------------------------------------------- ARP / SEQ
preset("Trance Gate Arp", "Arp / Seq", dict(s_wave1=0, s_wave2=0, s_tune2=0.12, s_lvl2=0.6, s_ftype=4, s_lpf=0.5, s_lpfRes=0.5, s_fltEnv=0.5, s_fD=0.18, s_fS=0.0,
      s_aD=0.25, s_aS=0.5, s_aR=0.2, s_arpOn=1, s_arpMode=0, s_arpOct=0, s_arpRate=0.79, s_assign=2, s_uniDetune=0.35, revMix=0.2, power=0.25), [])
preset("Arp Rain", "Arp / Seq", dict(s_wave1=2, s_wave2=2, s_oct2=3, s_lvl2=0.4, s_ftype=5, s_lpf=0.6, s_lpfRes=0.3, s_fltEnv=0.4, s_fD=0.2, s_fS=0.0,
      s_aD=0.3, s_aS=0.1, s_aR=0.4, s_arpOn=1, s_arpMode=4, s_arpOct=2, s_arpRate=0.7, s_arpGate=0.4, bbdMix=0.4, bbdSync=6, bbdFb=0.5, bbdPing=1, revMix=0.35, revDecay=0.75), [])
preset("Acid Lanes", "Arp / Seq", dict(s_wave1=0, s_wave2=0, s_tune2=0.08, s_lvl2=0.5, s_ftype=3, s_lpf=0.3, s_lpfRes=0.55, s_fltEnv=0.2, s_fD=0.2, s_fS=0.05,
      s_aD=0.3, s_aS=0.4, s_aR=0.1, s_drive=0.4, s_arpOn=1, s_arpMode=0, s_arpOct=1, s_assign=1, s_arpRate=0.68, s_arpGate=0.45, s_arpSwing=0.25, power=0.3, revMix=0.1),
       steps="1121101211211012",
       lanes=[(0, "MdLpf", 0.55, 0.2, [0.1, 0.5, 0.3, 0.9, 0.2, 0.7, 0.4, 1.0, 0.15, 0.6, 0.3, 0.8, 0.25, 0.55, 0.35, 0.95]), (1, "MdReso", 0.3, 0.0, [0, 0, 1, 0, 0, 1, 0, 0])])
preset("Pulse Climb", "Arp / Seq", dict(s_wave1=1, s_wave2=0, s_lvl2=0.4, s_tune2=0.1, s_ftype=4, s_lpf=0.45, s_lpfRes=0.4, s_fltEnv=0.5, s_fD=0.2, s_fS=0.0,
      s_aD=0.25, s_aS=0.1, s_aR=0.2, s_arpOn=1, s_arpMode=2, s_arpOct=2, s_arpRate=0.64, s_arpGate=0.5, revMix=0.25, revDecay=0.6, power=0.2), [("MsLfo1", "MdPw1", 0.3)])
preset("Random Dreams", "Arp / Seq", dict(s_wave1=2, s_wave2=2, s_fm=0.4, s_tune2=7, s_ftype=5, s_lpf=0.6, s_fltEnv=0.4, s_fD=0.25, s_fS=0.0, s_aD=0.3, s_aS=0.1, s_aR=0.4,
      s_arpOn=1, s_arpMode=2, s_arpOct=2, s_arpRate=0.61, s_assign=3, bbdMix=0.4, bbdPing=1, bbdSync=6, revMix=0.4, revDecay=0.7, power=0.2), [])
preset("Seq Pluck", "Arp / Seq", dict(s_wave1=1, s_wave2=0, s_tune2=0.1, s_lvl2=0.4, s_ftype=5, s_lpf=0.5, s_lpfRes=0.25, s_fltEnv=0.5, s_fD=0.22, s_fS=0.0,
      s_aD=0.2, s_aS=0.0, s_aR=0.2, s_arpOn=1, s_arpRate=0.66, s_arpGate=0.4, bbdMix=0.3, bbdSync=6, revMix=0.25, power=0.2),
       steps="2101121021011210", seq=([60, 67, 72, 64, 71, 67, 76, 62, 69, 74, 65, 72, 67, 79, 64, 71], None),
       lanes=[(0, "MdRevMix", 0.4, 0.0, [0.1, 0.1, 0.8, 0.1])])
preset("Ratchet Pluck", "Arp / Seq", dict(s_wave1=1, s_wave2=0, s_tune2=0.1, s_lvl2=0.4, s_ftype=3, s_lpf=0.5, s_lpfRes=0.35, s_fltEnv=0.5, s_fD=0.2, s_fS=0.0,
      s_aD=0.18, s_aS=0.0, s_aR=0.15, s_arpOn=1, s_arpMode=4, s_arpOct=2, s_assign=0, s_arpRate=0.6, s_arpGate=0.35, s_arpChance=0.8, revMix=0.3, revDecay=0.6, power=0.2),
       steps=("2101112110121011", "1121142111211421"), lanes=[(0, "MdRevMix", 0.5, 0.0, [0.1, 0.1, 0.4, 0.1, 0.9])])
preset("Polymeter Drift", "Arp / Seq", dict(s_wave1=0, s_wave2=1, s_lvl2=0.5, s_tune2=7, s_ftype=5, s_lpf=0.45, s_lpfRes=0.25, s_aA=0.2, s_aD=0.5, s_aS=0.5, s_aR=0.6,
      s_arpOn=2, s_arpMode=0, s_arpOct=2, s_arpRate=0.52, s_arpGate=0.8, revMix=0.35, revDecay=0.7, revShimmer=0.15, power=0.2),
       lanes=[(0, "MdLpf", 0.3, 0.2, [0.2, 0.7, 0.4, 0.9, 0.3]), (1, "MdPan", 0.5, 0.0, [0, 1, 0.5, 0.2, 0.8, 0.4, 0.9]), (2, "MdReso", 0.2, 0.0, [0, 0.5, 0, 1, 0, 0.3, 0, 0.8, 0.2])])
preset("Bass Sequence", "Arp / Seq", dict(s_wave1=0, s_oct1=1, s_wave2=1, s_oct2=1, s_lvl2=0.4, s_ftype=4, s_lpf=0.34, s_lpfRes=0.5, s_fltEnv=0.5, s_fD=0.2, s_fS=0.05,
      s_aD=0.3, s_aS=0.4, s_aR=0.1, s_arpOn=1, s_assign=1, s_arpRate=0.66, s_arpGate=0.55, power=0.3, compType=1, compAmt=0.35),
       steps="2112112121121121", seq=([33, 33, 45, 33, 36, 33, 48, 33, 31, 31, 43, 31, 38, 33, 45, 40], "0000100000001000"),
       lanes=[(0, "MdLpf", 0.45, 0.15, [0.3, 0.5, 0.8, 0.4, 0.6, 0.3, 0.9, 0.5])])
preset("Stutter Sequencer", "Arp / Seq", dict(s_wave1=0, s_wave2=1, s_tune2=0.1, s_lvl2=0.4, s_ftype=5, s_lpf=0.55, s_lpfRes=0.35, s_fltEnv=0.4, s_fD=0.25, s_fS=0.1,
      s_aD=0.3, s_aS=0.3, s_aR=0.2, s_arpOn=1, s_arpMode=2, s_arpOct=1, s_assign=0, s_arpRate=0.62, s_arpGate=0.6, revMix=0.2, revDecay=0.55, power=0.15,
      stutDiv=0, stutMode=0, stutFeel=0, stutLen=0.85, stutDecay=0.15, stutMix=1.0), steps="1111111111111111",
       lanes=[(0, "MdStutOn", 1.0, 0.0, [0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0]),
              (1, "MdStutDiv", 1.0, 0.0, [0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.67, 0.67, 0.5, 0.5, 0.5, 0.5, 0.83, 0.67, 0.5, 0.5]),
              (2, "MdStutMode", 1.0, 0.0, [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0.33, 0.67, 0, 0])])

# ----------------------------------------------------------------------- FX / TEXTURE
preset("Wind Machine", "FX / Texture", dict(s_lvl1=0.0, s_noise=1.0, s_ftype=4, s_lpf=0.55, s_lpfRes=0.5, s_l4Rate=0.3, s_l4Wave=4, s_l4Slew=0.5, s_aA=0.6, s_aS=1.0, s_aR=0.8,
      revMix=0.5, revDecay=0.8), [("MsLfo4", "MdLpf", 0.12), ("MsPolyAT", "MdLpf", 0.15)])
preset("Metal Rain", "FX / Texture", dict(s_wave1=0, s_wave2=1, s_tune2=5.5, s_ring=0.9, s_fm=0.4, s_lvl1=0.3, s_ftype=3, s_lpf=0.75, s_lpfRes=0.4, s_l3Rate=0.5,
      s_aD=0.3, s_aS=0.2, s_aR=0.5, revMix=0.4, revDecay=0.7, revShimmer=0.5), [("MsLfo3", "MdPitch2", 0.4)])
preset("Alien Choir", "FX / Texture", dict(s_wave1=2, s_wave2=2, s_fm=0.4, s_tune2=7, s_lvl2=0.5, s_ftype=5, s_semMorph=0.4, s_lpf=0.6, s_lpfRes=0.6, s_aA=0.6, s_aS=0.9, s_aR=0.8,
      revMix=0.5, revDecay=0.85, revSize=0.9, revShimmer=0.35), [("MsLfo1", "MdMorph", 0.25), ("MsLfo1", "MdFm", 0.15)])
preset("Thunder Drone", "FX / Texture", dict(s_wave1=0, s_oct1=0, s_wave2=1, s_oct2=0, s_lvl2=0.5, s_noise=0.3, s_ftype=4, s_lpf=0.25, s_lpfRes=0.4, s_aA=0.5, s_aS=1.0, s_aR=0.8,
      power=0.6, material=2, lightning=0.2, revMix=0.5, revDecay=0.9, revDamp=0.6), [("MsMac1", "MdDrive", 0.3)])
preset("Ring Wobble", "FX / Texture", dict(s_wave1=0, s_wave2=1, s_ring=0.8, s_tune2=5.5, s_lvl1=0.3, s_ftype=3, s_lpf=0.7, s_lpfRes=0.3, s_l2Rate=0.3, s_l2Wave=1,
      destroy=0.3, dSync=7, revMix=0.25), [("MsLfo2", "MdPw2", 0.6)])
preset("Sample And Hold Cloud", "FX / Texture", dict(s_wave1=2, s_wave2=0, s_lvl2=0.3, s_tune2=7, s_ftype=5, s_lpf=0.5, s_lpfRes=0.55, s_l1Wave=4, s_l1Rate=0.45, s_aA=0.4, s_aS=0.9, s_aR=0.7,
      revMix=0.45, revDecay=0.8, bbdMix=0.35, bbdSync=6, bbdFb=0.5, bbdPing=1), [("MsLfo1", "MdLpf", 0.2), ("MsLfo1", "MdPitch", 0.04)])
preset("Noise Riser", "FX / Texture", dict(s_lvl1=0.1, s_noise=0.7, s_ftype=4, s_lpf=0.2, s_lpfRes=0.7, s_fltEnv=0.9, s_fA=0.95, s_fD=0.9, s_fS=1.0, s_aA=0.5, s_aS=1.0, s_aR=0.5,
      revMix=0.4, revDecay=0.7), [("MsEnv", "MdReso", 0.05)])
preset("Dark Drone", "FX / Texture", dict(s_wave1=0, s_oct1=1, s_wave2=0, s_oct2=0, s_tune2=0.07, s_lvl2=0.7, s_ftype=4, s_lpf=0.28, s_lpfRes=0.5, s_fltEnv=0.1, s_aA=0.8, s_aS=1.0, s_aR=0.9,
      s_l1Rate=0.06, power=0.3, revMix=0.55, revDecay=0.95, revDamp=0.7, revSize=1.0), [("MsLfo1", "MdLpf", 0.1), ("MsLfo1", "MdPan", 0.3)])
preset("Cosmic Radio", "FX / Texture", dict(s_wave1=2, s_wave2=1, s_ring=0.4, s_fm=0.3, s_tune2=11, s_ftype=3, s_lpf=0.65, s_lpfRes=0.5, s_l1Wave=4, s_l1Rate=0.55, s_aA=0.3, s_aS=0.9, s_aR=0.6,
      destroy=0.35, dSync=7, dGrain=0.5, revMix=0.45, revDecay=0.8, revShimmer=0.3), [("MsLfo1", "MdFm", 0.2), ("MsLfo1", "MdRing", 0.2)])
