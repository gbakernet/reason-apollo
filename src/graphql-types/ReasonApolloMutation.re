module type InternalConfig = {let apolloClient: ApolloClient.generatedApolloClient;};

module MutationFactory = (InternalConfig:InternalConfig) => {
    external cast : string => {. "data": Js.Json.t, "loading": bool} = "%identity";
    [@bs.module] external gql : ReasonApolloTypes.gql = "graphql-tag";
    
    type state =
      | NotCalled
      | Loading
      | Loaded(Js.Json.t)
      | Failed(Js.Promise.error);

    type action =
      | Result(string)
      | Error(Js.Promise.error);

    let sendMutation = (~mutation, ~reduce) => {
      let _ =
      Js.Promise.(
        resolve(InternalConfig.apolloClient##mutate({
          "mutation": [@bs] gql(mutation##query),
          "variables": mutation##variables
        }))
        |> then_(
             (value) => {
               reduce(() => Result(value), ());
               resolve()
             }
           )
        |> catch(
             (error) => {
              reduce(() => Error(error), ());
              resolve()
             }
           )
      );
    };

    let component = ReasonReact.reducerComponent("ReasonApollo");
    let make = (children) => {
      ...component,
      initialState: () => NotCalled,
      reducer: (action, _state) =>
        switch action {
          | Result(result) => {
            let typedResult = cast(result)##data;
            ReasonReact.Update(Loaded(typedResult))
          }
          | Error(error) => ReasonReact.Update(Failed(error))
        },
      render: ({reduce, state}) => {
        let mutate = (mutationFactory) => {
          sendMutation(~mutation=mutationFactory, ~reduce);
        };
        children(mutate, state);
      }
    };
  };
