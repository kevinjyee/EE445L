When wiring up the DAC8551 to SSI ports on the TM4C, I was unsure if SYNCinv should be wired directly to SSI_FS or if the signal from SSI_FS should be inverted first. FIGURE THIS OUT.
Other things to consider: 1) Need to add second logic analyzer.
		a) Logic analyzer can be used as a backup for damaged tracks.
		b) Could also hook up unused port to LA and toggle during ISR
			to use as debugging heartbeat.
	2) Need to modularize hardware.
	3) Remove hardware/add another DAC?
