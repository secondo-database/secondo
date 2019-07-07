import { Response, Request, NextFunction } from "express";
export class ConfigureHeadersForApp {
    public static configureHeaders(req: Request, res: Response, next: NextFunction) {
        res.header('Access-Control-Allow-Origin', '*');
        res.header('Access-Control-Allow-Headers', '*, Content-Type, Authorization');
        res.header('Access-Control-Allow-Methods', 'GET, DELETE, POST, PUT, OPTIONS');
        res.header('Access-Control-Max-Age', '86400');
        res.header('Cache-Control', 'private, no-cache, no-store, must-revalidate');
        res.header('Expires', '-1');
        res.header('Pragma', 'no-cache');
        next();
    }
}