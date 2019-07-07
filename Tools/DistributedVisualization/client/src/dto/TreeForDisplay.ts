export interface TreeForDisplayDTO {
    name: string;
    operator?: string;
    function?: string;
    observable?: boolean;
    children?: TreeForDisplayDTO[];
    nowWorkingOn?: boolean;
    ready?: boolean;
    operand?: boolean;
    result?: string;
}
