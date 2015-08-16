
Stochastic events
=================

The parameters defining the geological events are, by their very nature,
highly uncertain. In addition to these uncertainties, the kinematic
approach by itself is only a limiting approximation. The picture that we
obtain from the kinematic forword model is therefore a very
overconfident repsresentation of reality - an aspect that (hopefully)
everyone using Noddy is aware of...

In order to respect the vast nature of these uncertainties, we introduce
here an adapted version of the standard geological events defined in
Noddy: the definition of a stochastic event:

A stochastic event is geological event in the Noddy history that has an
uncertainty associated to it in such a way that, recomputing the
geolgoical history will result in a different result each time (note:
the definition is borrowed from the notion of stochastic events in
probabilistic programming, see for example pymc).


Definition of stochastic events
-------------------------------

We start, as before, with a pre-defined geological noddy history for
simplicity:

