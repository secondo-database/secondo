export interface TreeDTO {
    result_type: string;
    operator: string;
    operators_num_of_functions: number;
    sons: number[];
    constant: boolean;
    object: boolean;
    function: boolean;
    info: string;
    no_of_sons: number;
    average_progress: number;
}
