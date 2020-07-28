# needs pandas and altair module
# pip install pandas
# pip install altair vega_datasets

import sys
import altair as alt
import pandas as pd

def main():
    if len(sys.argv) != 4:
        print("Error: first argument must be the input file, second one the xaxis  \
               start point and the third argument the xaxis end point")
        return

    inputFile = sys.argv[1] 
    startS = sys.argv[2] #550000
    endS = sys.argv[3] #800000

    #load data
    source = pd.read_csv(inputFile, sep='\t')

    #load
    barc = alt.Chart(source).mark_bar(stroke='black', fillOpacity=0.4).encode(
        alt.X('start',
            scale=alt.Scale(
                domain=(startS, endS),
                clamp=True
            ),
            axis=alt.Axis(title="Zeit in Mikrosekunden")
        ),
        x2='end',
        y=alt.Y('thread', axis=alt.Axis(title="Thread")),
        color=alt.Color('context', scale=alt.Scale(scheme='set3'), legend=alt.Legend(title="Kontextinstanz", orient="bottom"))
    ).properties(
        width=500,
        height=200
    )


    text = alt.Chart(source).mark_text(align='left', baseline='middle', dx=5, color="white",  fontWeight="bold").encode(
        alt.X('start',
            scale=alt.Scale(
                domain=(startS, endS),
                clamp=True
            ),
            axis=alt.Axis(title="Zeit in Mikrosekunden")
        ),
        x2='end',
        y='thread',
        text="context"
    ).properties(
        width=500,
        height=200
    )

    #layer the two gantt plots
    chart = barc #+ text

    #save them to rendered html
    chart.save('chart.html')

if __name__ == "__main__":
    main()

