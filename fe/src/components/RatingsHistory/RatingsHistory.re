open Svc;
open EloTypes;
open Helpers;
[%bs.raw {|require('./RatingsHistory.scss')|}];

type state = {
  ratingsHistory: option(list(ratingHistory)),
  inputCode: string,
  warning: bool,
};

type action =
  | GetHistory(list(user))
  | SetHistory(list(ratingHistory))
  | ChangeCode(string);

let component = ReasonReact.reducerComponent("Stats");

let initialState = () => {
  ratingsHistory: None,
  inputCode: "",
  warning: false,
};

let getHistorySvc = (state, users) => {
  let userNid = getUserNidFromCode(state.inputCode, users);

  ReasonReact.UpdateWithSideEffects(
    {...state, warning: false},
    self =>
      Js.Promise.(
        svcGet("ratings_history/" ++ string_of_int(userNid))
        |> then_(json =>
             DecodeRatingsHistory.ratingsHistoryDec(json) |> resolve
           )
        |> then_(result => self.send(SetHistory(result)) |> resolve)
      )
      |> ignore,
  );
};

let getHistoryReducer = (state: state, users: list(user)) => {
  let userExist =
    List.exists(user => compareCodes(user.code, state.inputCode), users);

  userExist ?
    getHistorySvc(state, users) :
    ReasonReact.Update({...state, warning: true});
};

let reducer = (action, state) =>
  switch (action) {
  | GetHistory(users) => getHistoryReducer(state, users)
  | ChangeCode(code) => ReasonReact.Update({...state, inputCode: code})
  | SetHistory(ratingsHistory) =>
    ReasonReact.Update({...state, ratingsHistory: Some(ratingsHistory)})
  };

let make = (~users, _children) => {
  ...component,
  initialState,
  reducer,
  render: self =>
    <div className="ratingsHistory">
      {
        self.state.warning ?
          <div className="warning">
            {ReasonReact.string("Player doesn't exist")}
          </div> :
          ReasonReact.null
      }
      <form
        className="topBar"
        onSubmit={
          event => {
            event |> ReactEvent.Form.preventDefault;
            self.send(GetHistory(users));
          }
        }>
        <table>
          <tbody>
            <tr>
              <td>
                <input
                  placeholder="code"
                  onChange={
                    event =>
                      self.send(
                        ChangeCode(GameResult.valueFromEvent(event)),
                      )
                  }
                />
              </td>
              <td> <button> {ReasonReact.string("Get stats")} </button> </td>
            </tr>
          </tbody>
        </table>
      </form>
      {
        switch (self.state.ratingsHistory) {
        | None => ReasonReact.null
        | Some(ratingsHistory) =>
          List.length(ratingsHistory) === 0 ?
            <div className="warning">
              {"This player has no games" |> ReasonReact.string}
            </div> :
            <RatingsHistoryTable ratingsHistory />
        }
      }
    </div>,
};