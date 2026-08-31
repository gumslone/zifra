/*
 * Melody preview — a WebAudio square-wave approximation of the piezo,
 * playing the same note tables the firmware ships.
 */
(function (Z) {
    'use strict';

    let audioCtx = null;
    let previewOsc = null;

    Z.previewMelody = (index) => {
        if (previewOsc) {
            try { previewOsc.stop(); } catch (e) { /* already stopped */ }
            previewOsc = null;
        }
        const AC = window.AudioContext || window.webkitAudioContext;
        if (!AC) return;
        if (!audioCtx) audioCtx = new AC();
        if (audioCtx.resume) audioCtx.resume();

        const osc = audioCtx.createOscillator();
        const gain = audioCtx.createGain();
        osc.type = 'square';
        osc.connect(gain);
        gain.connect(audioCtx.destination);

        const melody = Z.MELODIES[index] || Z.MELODIES[0];
        let t = audioCtx.currentTime;
        gain.gain.setValueAtTime(0, t);
        for (let loop = 0; loop < 2; loop++) {
            melody.notes.forEach((note) => {
                if (note[0] > 0) {
                    osc.frequency.setValueAtTime(note[0], t);
                    gain.gain.setValueAtTime(0.08, t);
                } else {
                    gain.gain.setValueAtTime(0, t);
                }
                t += note[1] / 1000;
            });
        }
        gain.gain.setValueAtTime(0, t);
        osc.start();
        osc.stop(t + 0.05);
        previewOsc = osc;
    };
})(window.ZIFRA);
