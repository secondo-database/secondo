interface Props {
  time: number; // seconds into the domain (0..duration)
  duration: number; // seconds
  t0: number; // POSIX seconds of domain start (for absolute labels)
  playing: boolean;
  speed: number;
  onPlay: (playing: boolean) => void;
  onSeek: (time: number) => void;
  onSpeed: (speed: number) => void;
}

const SPEEDS = [30, 60, 120, 300, 600];

function fmtClock(posixSeconds: number): string {
  // Absolute wall-clock (UTC) HH:MM:SS of the domain instant.
  return new Date(posixSeconds * 1000).toISOString().slice(11, 19);
}

export function Timeline({
  time,
  duration,
  t0,
  playing,
  speed,
  onPlay,
  onSeek,
  onSpeed,
}: Props) {
  return (
    <div className="timeline">
      <button
        className="tl-play"
        onClick={() => onPlay(!playing)}
        title={playing ? "Pause" : "Play"}
      >
        {playing ? "❚❚" : "▶"}
      </button>
      <span className="tl-clock">{fmtClock(t0 + time)}</span>
      <input
        className="tl-range"
        type="range"
        min={0}
        max={duration}
        step={Math.max(duration / 1000, 0.001)}
        value={time}
        onChange={(e) => onSeek(Number(e.target.value))}
      />
      <span className="tl-clock">{fmtClock(t0 + duration)}</span>
      <select
        className="tl-speed"
        value={speed}
        onChange={(e) => onSpeed(Number(e.target.value))}
        title="Playback speed (simulated seconds per real second)"
      >
        {SPEEDS.map((s) => (
          <option key={s} value={s}>
            {s}×
          </option>
        ))}
      </select>
    </div>
  );
}
