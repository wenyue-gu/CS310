import java.util.*;
public class RaftNode {

	private RaftLog mLog;
	private Timer mTimer;
	private AppendEntryRequester mAppendEntryRequester;
	private VoteRequester mVoteRequester;
	private int mNumServers;
	private int mNodeServerNumber;
	private RaftMode mCurrentMode;
	private int mTerm;
	private int mCommitIndex;
	private int votedFor = -1;
	private int[] prev;
	private int[] match;


	/**
	 * RaftNode constructor, called by test harness.
	 * @param log this replica's local log, pre-populated with entries
	 * @param timer this replica's local timer
	 * @param sendAppendEntry call this to send RPCs as leader and check responses
	 * @param sendVoteRequest call this to send RPCs as candidate and check responses
	 * @param numServers how many servers in the configuration (numbered starting at 0)
	 * @param nodeServerNumber server number for this replica (this RaftNode object)
	 * @param currentMode initial mode for this replica
	 * @param term initial term (e.g., last term seen before failure).  Terms start at 1.
	 **/
	public RaftNode(
			RaftLog log,
			Timer timer,
			AppendEntryRequester sendAppendEntry,
			VoteRequester sendVoteRequest,
			int numServers,
			int nodeServerNumber,
			RaftMode currentMode,
			int term) {
		mLog = log;
		mTimer = timer;
		mAppendEntryRequester = sendAppendEntry;
		mVoteRequester = sendVoteRequest;
		mNumServers = numServers;
		mNodeServerNumber = nodeServerNumber;
		mCurrentMode = currentMode;
		mTerm = term;
		mCommitIndex = -1;
		votedFor = -1;

	}

	public RaftMode getCurrentMode() {
		return mCurrentMode;
	}

	public int getCommitIndex() {
		return mCommitIndex;
	}

	public int getTerm() {
		return mTerm;
	}

	public RaftLog getCurrentLog() {
		return mLog;
	}

	public int getServerId() {
		return mNodeServerNumber;
	}

	public int getLastApplied() {
		return mLog.getLastEntryIndex();
	}

	public boolean votedInTerm(int term) {
	// Return whether or not this RaftNode has voted in specified term.
	// Note: called only for current term or future term.
		if(mTerm==term){
			return (votedFor!=-1);
		}
	   	return false;
	}

	/**
	 * @param candidateTerm candidate's term
	 * @param candidateID   candidate requesting vote
	 * @param lastLogIndex  index of candidate's last log entry
	 * @param lastLogTerm   term of candidate's last log entry
	 * @return 0, if server votes for candidate; otherwise, server's current term
	 */
	public int receiveVoteRequest(int candidateTerm,
								  int candidateID,
								  int lastLogIndex,
								  int lastLogTerm) {
		if(candidateTerm<mTerm) return mTerm;
		if(mTerm<candidateTerm){
			if(mCurrentMode==RaftMode.CANDIDATE || mCurrentMode==RaftMode.LEADER) 
				mCurrentMode = RaftMode.FOLLOWER;
			votedFor = -1;
		}
		mTerm = candidateTerm;
		if((votedFor==-1 || votedFor==candidateID)){
			if (mLog.getLastEntryTerm() > lastLogTerm){
				return mTerm;
			}
			else if (mLog.getLastEntryTerm() < lastLogTerm || mLog.getLastEntryIndex()<=lastLogIndex){
				votedFor = candidateID;
				return 0;
			}
		}
		return mTerm;
	}

	/**
	 * @param leaderTerm   leader's term
	 * @param leaderID     current leader
	 * @param prevLogIndex index of log entry before entries to append
	 * @param prevLogTerm  term of log entry before entries to append
	 * @param entries      entries to append (in order of 0 to append.length-1)
	 * @param leaderCommit index of highest committed entry
	 * @return 0, if follower accepts; otherwise, follower's current term
	 */
	public int receiveAppendEntry(int leaderTerm,
								  int leaderID,
								  int prevLogIndex,
								  int prevLogTerm,
								  Entry[] entries,
								  int leaderCommit) {
		System.out.println("inside receive append entry");
		if(leaderTerm<mTerm) return mTerm;
		System.out.println("checked that term matches");

		mTerm = leaderTerm;
		if(mCurrentMode==RaftMode.LEADER || mCurrentMode==RaftMode.CANDIDATE) mCurrentMode=RaftMode.FOLLOWER;

		System.out.println("update term and swap mode");

		if(prevLogIndex!=-1){
			Entry e = mLog.getEntry(prevLogIndex);
			if(e==null || e.term!=prevLogTerm) return mTerm;
		}

		System.out.println("last one match, inserting");

		mLog.insert(entries, prevLogIndex);
		if(leaderCommit>mCommitIndex)
			mCommitIndex = Math.min(leaderCommit, mLog.getLastEntryIndex());
		return 0;
	}

	public int handleTimeout() {
		System.out.println("Inside time out");
		if(mCurrentMode==RaftMode.LEADER){
			System.out.println("Inside leader");
			List <AppendResponse> list = mAppendEntryRequester.getResponses(mTerm);
			for(AppendResponse a:list){
				int i = a.responderId;
				if(a.success){
					prev[i] = Math.min(prev[i]+1,mLog.getLastEntryIndex());
					match[i] = prev[i]+1;
					System.out.println( "server"+i+" match" +match[i]);
				}
				else{
					if(a.term>mTerm){
						mCurrentMode = RaftMode.FOLLOWER;
						mTerm = a.term;
						mTimer.resetTimer();
						return 0;
					}
					prev[i] = prev[i]-1;
				}
			}
			int[] temp = match;
			Arrays.sort(temp);
			int half = (mNumServers)/2;
			int n = match[half];
			if(n>mCommitIndex){
				Entry e = mLog.getEntry(n);
				if(e!=null){
					//System.out.println(e.term);
					//if(e.term==mTerm) 
					mCommitIndex = n;
					for(int i = 0; i<mNumServers; i++){
						prev[i] = n;
					}
				}
			}
			requestAppend();
		}
		else if(mCurrentMode==RaftMode.CANDIDATE){
			List <VoteResponse> list = mVoteRequester.getResponses(mTerm);
			boolean cont = true;
			for(VoteResponse v:list){
				if(v.term>mTerm){
					mCurrentMode = RaftMode.FOLLOWER;
					mTerm = v.term;
					mTimer.resetTimer();
					return 0;
				}
			}
			int suc = mVoteRequester.countYesResponses(mTerm);
			if(suc>=0.5*(mNumServers)){
				mCurrentMode=RaftMode.LEADER;
				System.out.println("leader now");

				prev = new int[mNumServers];
				match = new int[mNumServers];
				for(int i = 0; i<prev.length; i++){
					prev[i] = mLog.getLastEntryIndex();
					match[i] = -1;
				}
				requestAppend();
			}
			else{
				mCurrentMode=RaftMode.CANDIDATE;
				mTerm+=1;
				votedFor = mNodeServerNumber;
				requestVotes();
			}
		}
		else{
			mCurrentMode=RaftMode.CANDIDATE;
			mTerm+=1;
			votedFor = mNodeServerNumber;
			requestVotes();
			// mVoteRequester.addVoteResponseForTerm(
			// 	mTerm,
			// 	new VoteResponse(mNodeServerNumber, mNodeServerNumber, true, mTerm)
			// );
		}
		mTimer.resetTimer();
		return 0;
	}

	/**
	 *  Private method shows how to issue a round of RPC calls.
	 *  Responses come in over time: after at least one timer interval, call mVoteRequester to query/retrieve responses.
	 */
	private void requestVotes() {
		for (int i = 0; i < mNumServers; i++) {
			if (i != mNodeServerNumber) {
				mVoteRequester.send(i, mTerm, mNodeServerNumber, mLog.getLastEntryIndex(), mLog.getLastEntryTerm());
			}
		}
	}

	private void requestAppend() {
		System.out.println("term:" + mTerm + " request append");
		for (int i = 0; i < mNumServers; i++) {
			if (i != mNodeServerNumber) {
				System.out.println("i" + i + " have prev ind" + prev[i]);
				Entry e = mLog.getEntry(prev[i]);
				int term;
				if(e == null) term = -1;
				else term = e.term;
				mAppendEntryRequester.send(i, mTerm, mNodeServerNumber, prev[i], term, new Entry[]{}, mCommitIndex);
			}
		}
	}

}
