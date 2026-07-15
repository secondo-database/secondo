import { useEffect, useRef, useState } from "react";

/** Drives a `time` value in [0, duration] via requestAnimationFrame.
 *
 * This replaces the HoeseViewer's Swing `javax.swing.Timer` animation loop
 * (`AnimCtrlListener`): a single clock the moving-object layers read from.
 * `speed` is simulated seconds advanced per real second. Playback loops.
 */
export function useAnimator(duration: number) {
  const [time, setTime] = useState(0);
  const [playing, setPlaying] = useState(false);
  const [speed, setSpeed] = useState(120);
  const raf = useRef<number | undefined>(undefined);
  const last = useRef<number | undefined>(undefined);

  // Reset to the start whenever a new dataset (new duration) arrives.
  useEffect(() => {
    setTime(0);
    setPlaying(duration > 0);
  }, [duration]);

  useEffect(() => {
    if (!playing || duration <= 0) return;
    last.current = performance.now();
    const tick = (now: number) => {
      const dt = (now - (last.current ?? now)) / 1000;
      last.current = now;
      setTime((t) => {
        const nt = t + dt * speed;
        return nt >= duration ? nt % duration : nt; // loop
      });
      raf.current = requestAnimationFrame(tick);
    };
    raf.current = requestAnimationFrame(tick);
    return () => {
      if (raf.current !== undefined) cancelAnimationFrame(raf.current);
    };
  }, [playing, speed, duration]);

  return { time, setTime, playing, setPlaying, speed, setSpeed };
}
