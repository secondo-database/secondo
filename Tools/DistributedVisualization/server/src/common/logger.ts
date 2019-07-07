/**
 * Provides logging for other components
 */
import { format, createLogger, transports, addColors, Logger } from "winston";

addColors({
    trace: "magenta",
    input: "grey",
    verbose: "cyan",
    prompt: "grey",
    debug: "blue",
    info: "green",
    data: "grey",
    help: "cyan",
    warn: "yellow",
    error: "red"
});

const customFormat = format.printf(info => {
    return `${info.level}: ${info.message} | [${info.label}] | ${info.timestamp}`;
});

module.exports = function withCaller(caller: NodeModule): Logger {
    return createLogger({
        transports: [
            new transports.Console({
                format: format.combine(
                    format.label({ label: caller.filename.replace(process.cwd(), "") }),
                    format.timestamp(),
                    format.prettyPrint(),
                    format.colorize(),
                    format.splat(),
                    format.simple(),
                    customFormat
                ),
                level: "debug",
                silent: false,
            })
        ]
    });
};
