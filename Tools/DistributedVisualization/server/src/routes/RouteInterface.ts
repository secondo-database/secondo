import { RequestHandler } from "express";

export interface RouteInterface {
    method: string;
    path: string;
    validators: RequestHandler[];
    authenticators: RequestHandler[];
    handler: RequestHandler;
}