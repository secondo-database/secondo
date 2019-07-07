export class JournalService {
    public static calculateElapsedTime(
        timestampStarted: string,
        timestampFinished: string,
    ): string {
        if (timestampFinished !== "") {
            const diff: number =
                Number.parseInt(timestampFinished, 10) -
                Number.parseInt(timestampStarted, 10);
            return this.generateTimeString(diff);
        } else {
            const now: number = Math.floor(new Date().getTime() / 1000);
            const diff: number = now - Number.parseInt(timestampStarted, 10);
            return this.generateTimeString(diff);
        }
    }

    private static generateTimeString(diff: number): string {
        const h: number = (diff - (diff % 3600)) / 3600;
        const m: number = ((diff % 3600) - ((diff % 3600) % 60)) / 60;
        const s: number = diff - h * 3600 - m * 60;
        return (
            (h < 10 ? "0" + h : h) +
            ":" +
            (m < 10 ? "0" + m : m) +
            ":" +
            (s < 10 ? "0" + s : s)
        );
    }
}
